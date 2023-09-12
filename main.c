#include <ctype.h>

#include "rpi.h"
#include "util.h"
#include "marklin.h"
#include "parser.h"
#include "string.h"
#include "cbuf.h"

#define NUMBER_OF_TRAINS 80

static const char* PROMPT_ANCHOR = "\033[25;5H";
static const char* SENSORS_ANCHOR = "\033[8;62H";
                                                   
// Formats system time into human readable string
void fmt_time(uint64_t time) {
  
  unsigned int f_tenths = time % 1000000 / 100000;
  unsigned int secs = time / 1000000;
  unsigned int f_secs = secs % 60;
  unsigned int f_min = secs / 60;

  uart_printf(CONSOLE, "%sTIME [%u] %u:%u:%u0 ", ANSI_MOVE("0", "0"), time, f_min, f_secs, f_tenths);

}

// Keeps track of the last time each event was ran
typedef struct {
  // Last time the timer was updated
  uint32_t timer;
  // Last time the state of the sensors were dumped
  uint32_t sensor;
  uint32_t write;
  uint32_t read;
  uint32_t stop_times[NUMBER_OF_TRAINS];
  uint32_t reverse_times[NUMBER_OF_TRAINS];
} TimerEvents;

TimerEvents
timerevents_new(void)
{
  TimerEvents timer_events = {
    .timer = 0,
    .sensor = 0,
    .write = 0,
    .read = 0,
    .stop_times = {0},
  };
  return timer_events;
}


int kmain() {

  // used to track speed of each train
  uint32_t train_state[NUMBER_OF_TRAINS] = {0};

  // initialize both console and marklin uarts
  uart_init();

  // not strictly necessary, since line 1 is configured during boot
  // but we'll configure the line anyways, so we know what state it is in
  // set the line control registers: 8 bit, no parity, 1 stop bit, FIFOs enabled
  // UART_LCRH_WLEN_HIGH | UART_LCRH_WLEN_LOW | UART_LCRH_FEN

  uart_config_and_enable(CONSOLE, 115200, 0x70);

  // set the line control registers: 8 bit, no parity, 2 stop bit, FIFOs enabled
  // UART_LCRH_WLEN_HIGH | UART_LCRH_WLEN_LOW | UART_LCRH_FEN | UART_LCRH_STP2
  uart_config_and_enable(MARKLIN, 2400, 0x78);

  TimerEvents timer_events = timerevents_new();
  SwitchTable switch_table = switchtable_new();

  uint64_t timer_value = 0;

  String line = string_new();

  uart_printf(CONSOLE, "%s%s%s", ANSI_CLEAR, ANSI_ORIGIN, ANSI_HIDE);

  // print a cool banner
  uart_printf(CONSOLE, "\r\n");
  uart_printf(CONSOLE, "     ~~~~ ____   |~~~~~~~~~~~~~|   |~~~~~~~~~~~~~|   |~~~~~~~~~~~~~|\r\n");
  uart_printf(CONSOLE, "    Y_,___|[]|   | MARKLIN CTL |   |  CS452 F23  |   | Daniel  Liu |\r\n");
  uart_printf(CONSOLE, "   {|_|_|_|PU|_,_|_____________|-,-|_____________|-,-|_____________|\r\n");
  uart_printf(CONSOLE, "  //oo---OO=OO     OOO     OOO       000     000       000     000  \r\n");
  uart_printf(CONSOLE, "\r\n");
  uart_printf(CONSOLE, "╭─[console]────────────────────────────────────────────────┬─[sensors]──────────╮\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          ├─[switches]─────────┤\r\n");
  uart_printf(CONSOLE, "│                                                          │ 01 X     12 X      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 02 X     13 X      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 03 X     14 X      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 04 X     15 X      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 05 X     16 X      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 06 X     17 X      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 07 X     18 X      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 08 X               │\r\n");
  uart_printf(CONSOLE, "│╭────────────────────────────────────────────────────────╮│ 09 X               │\r\n");
  uart_printf(CONSOLE, "││>                                                       ││ 10 X               │\r\n");
  uart_printf(CONSOLE, "│╰────────────────────────────────────────────────────────╯│ 11 X               │\r\n");
  uart_printf(CONSOLE, "╰──────────────────────────────────────────────────────────┴────────────────────╯\r\n");

  // 80 wide, console goes up to 60

  bool line_changed = true; // flag used to decide when to redraw the line
  uint32_t cmd_log_length = 0; // number of lines of command log text we have printed out
  uint32_t sensor_log_length = 0; // number of lines of sensor log text we have printed out
  uint32_t sensor_bytes_expecting = 0; // number of bytes we are expecting to read from sensors

  CBuf out_stream = cbuf_new();

  while (1) {

    timer_value = timer_get();

    // should the timer be updated
    if (timer_value - timer_events.timer > 10000) {
      timer_events.timer = timer_value;
      fmt_time(timer_value);
    }

    // poll switches
    if (timer_value - timer_events.sensor > 1000000 && sensor_bytes_expecting == 0) {
      timer_events.sensor = timer_value;
      marklin_dump_s88(&out_stream);
      sensor_bytes_expecting = 10; // 5 sensors with 2 bytes each
    }

    // check if terminal has a byte
    unsigned char c = 0;
    uart_getc_poll(CONSOLE, &c);

    // check if sensors has a byte
    if (timer_value - timer_events.read > 100000 && sensor_bytes_expecting > 0) {
      timer_events.read = timer_value;

      unsigned char sensor_byte = 0;
      // if we had data
      if (uart_getc_poll(MARKLIN, &sensor_byte) == 0) {
        uint8_t triggered = switchtable_write(&switch_table, 10-sensor_bytes_expecting, sensor_byte);
        uart_printf(CONSOLE, "\033[%u;62H", 8 + sensor_log_length);

        char sensor_group[2] = {(10-sensor_bytes_expecting) / 2 + 'A', 0};

        for (unsigned int i = 0; i < 8; ++i) {

          // figure out name of sensor that was triggered
          unsigned int sensor_num = (8-i) + (((10-sensor_bytes_expecting) % 2 == 1) ? 8 : 0); // earlier byte is high

          if (((triggered >> i) & 0x1) == 0x1) {
            uart_printf(CONSOLE, "%s:%u\r\n", sensor_group, sensor_num);
            ++sensor_log_length;
          }
        }
        --sensor_bytes_expecting;
        /* uart_printf(CONSOLE, "\033[20;0H\033[K sensor bytes %u, got %u", sensor_bytes_expecting, sensor_byte); */
      }
    }

    if (isalnum(c) || isblank(c)) {
      string_pushc(&line, c);
      line_changed = true;
    }
    else if (c == 0x0d) {
      // enter is pressed

      uart_printf(CONSOLE, "\033[%u;2H", 8 + cmd_log_length);

      // parse the line
      ParserResult parser_result = parse_command(string_data(&line));

      if (parser_result._type == PARSER_RESULT_TRAIN_SPEED) {
        uint32_t train = parser_result._data.train_speed.train;
        uint32_t speed = parser_result._data.train_speed.speed;
        marklin_train_ctl(&out_stream, train, speed);
        train_state[train] = speed;
        uart_printf(CONSOLE, "sending command for train %u at speed %u", train, speed);
        ++cmd_log_length;
      }
      else if (parser_result._type == PARSER_RESULT_REVERSE) {
        uint32_t train = parser_result._data.reverse.train;

        marklin_train_ctl(&out_stream, train, SPEED_STOP);

        // start timer for train to reverse direction
        timer_events.stop_times[train] = timer_value;

        uart_printf(CONSOLE, "reversing direction for train %u", train);
        ++cmd_log_length;
      }
      else if (parser_result._type == PARSER_RESULT_SWITCH) {
        uint32_t switch_id = parser_result._data.switch_control.switch_id;
        SwitchMode switch_mode = parser_result._data.switch_control.switch_mode;

        marklin_switch_ctl(&out_stream, switch_id, switch_mode);

        uart_printf(CONSOLE, "setting switch %u to mode %u", switch_id, switch_mode);
        ++cmd_log_length;
      }
      else if (parser_result._type == PARSER_RESULT_QUIT) {
        uart_printf(CONSOLE, "%s%s", ANSI_CLEAR, ANSI_ORIGIN);
        break;
      }
      else {
        uart_printf(CONSOLE, "invalid command");
        ++cmd_log_length;
      }

      string_clear(&line);
      line_changed = true;

    }
    else if (c == 0x08) {
      // backspace is pressed
      string_popc(&line);
      line_changed = true;
    }

    // reverse direction for train
    for (unsigned int i = 0; i < NUMBER_OF_TRAINS; ++i) {

      // timer for when we can reverse train
      if (timer_events.stop_times[i] > 0) {
        if (timer_value - timer_events.stop_times[i] > 2000000) {
          marklin_train_ctl(&out_stream, i, SPEED_REVERSE);
          timer_events.stop_times[i] = 0;
          timer_events.reverse_times[i] = timer_value;
        }
      }

      // timer for when we can set train back to original speed
      if (timer_events.reverse_times[i] > 0) {
        if (timer_value - timer_events.reverse_times[i] > 500000) {
          marklin_train_ctl(&out_stream, i, train_state[i]);
          timer_events.reverse_times[i] = 0;
        }
      }

    }

    if (line_changed) {
      uart_printf(CONSOLE, "%s                                                      ", PROMPT_ANCHOR);
      uart_printf(CONSOLE, "%s%s", PROMPT_ANCHOR, string_data(&line));
      line_changed = false;
    }

    /* uart_printf(CONSOLE, "\033[30;0H\033[K cbuf len %u", cbuf_len(&out_stream)); */

    // write to MARKLIN if we can
    if (timer_value - timer_events.write > 1000) {
      timer_events.write = timer_value;
      if (cbuf_len(&out_stream) > 0) {
        uint8_t byte = cbuf_front(&out_stream);
        if (uart_try_putc(MARKLIN, byte) == 0) cbuf_pop(&out_stream);
      }
    }



  }

  // U-Boot displays the return value from main - might be handy for debugging
  return 0;
}
