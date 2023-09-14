#include <ctype.h>

#include "rpi.h"
#include "util.h"
#include "marklin.h"
#include "parser.h"
#include "string.h"
#include "cbuf.h"
#include "ui.h"

#define NUMBER_OF_TRAINS 80

#define T_TIMER  100000
#define T_SENSOR 100000 
#define T_WRITE 100000
#define T_READ 30000 

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
  uint32_t dev; // used for debugging
} TimerEvents;

TimerEvents
timerevents_new(void)
{
  // stagger timers to avoid long loop when program starts
  TimerEvents timer_events = {
    .timer = 100,
    .sensor = 200,
    .write = 300,
    .read = 400,
    .stop_times = {0},
    .dev = 0,
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

  setup_ui();

  draw_static_ui();

  // 80 wide, console goes up to 60

  bool line_changed = true; // flag used to decide when to redraw the line
  uint32_t cmd_log_length = 0; // number of lines of command log text we have printed out
  uint32_t sensor_log_length = 0; // number of lines of sensor log text we have printed out
  uint32_t sensor_bytes_expecting = 0; // number of bytes we are expecting to read from sensors

  uint32_t worst_main_loop_time = 0; // used for timing the runtime of the main loop

  CBuf out_stream = cbuf_new();

  while (1) {

    timer_value = timer_get();

    // should the timer be updated
    if (timer_value - timer_events.timer > T_TIMER) {
      timer_events.timer = timer_value;
      draw_time(timer_value);
    }

    // poll switches
    if (timer_value - timer_events.sensor > T_SENSOR && sensor_bytes_expecting == 0) {
      timer_events.sensor = timer_value;
      marklin_dump_s88(&out_stream);
      sensor_bytes_expecting = 10; // 5 sensors with 2 bytes each
    }

    // check if terminal has a byte
    unsigned char c = 0;
    uart_getc_poll(CONSOLE, &c);

    // check if sensors has a byte
    if (timer_value - timer_events.read > T_READ && sensor_bytes_expecting > 0) {
      timer_events.read = timer_value;

      unsigned char sensor_byte = 0;
      // if we had data
      if (uart_getc_poll(MARKLIN, &sensor_byte) == 0) {

        // wipe pane if full
        if (sensor_log_length >= SENSOR_LOG_MAX_ENTRIES) {
          clear_sensor_window();
          sensor_log_length = 0;
        }

        uint8_t triggered = switchtable_write(&switch_table, 10-sensor_bytes_expecting, sensor_byte);
        

        char sensor_group[2] = {(10-sensor_bytes_expecting) / 2 + 'A', 0};

        for (unsigned int i = 0; i < 8; ++i) {

          // figure out name of sensor that was triggered
          unsigned int sensor_num = (8-i) + (((10-sensor_bytes_expecting) % 2 == 1) ? 8 : 0); // earlier byte is high

          if (((triggered >> i) & 0x1) == 0x1) {
            uart_printf(CONSOLE, "\033[%u;%uH", 10 + sensor_log_length % 7, 62 + (sensor_log_length / 7) * 4);
            uart_printf(CONSOLE, "%s%u", sensor_group, sensor_num);
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

      // wipe terminal if we fill up terminal
      if (cmd_log_length >= CONSOLE_MAX_LINES) {
        clear_command_window();
        cmd_log_length = 0;
      }

      uart_printf(CONSOLE, "\033[%u;2H", 10 + cmd_log_length);

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

        // update UI
        uart_printf(CONSOLE, "\033[%u;%uH", 18 + (switch_id-1) % 11, 65 + ((((switch_id-1) / 11) == 0) ? 0 : 9));
        if (switch_mode == SWITCH_MODE_CURVED) {
          uart_printf(CONSOLE, "C");
        } else {
          uart_printf(CONSOLE, "S");
        }

      }
      else if (parser_result._type == PARSER_RESULT_STOP) {
        marklin_stop(&out_stream);
        uart_printf(CONSOLE, "stopping marklin");
        ++cmd_log_length;
      }
      else if (parser_result._type == PARSER_RESULT_GO) {
        marklin_go(&out_stream);
        uart_printf(CONSOLE, "starting marklin");
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
        if (timer_value - timer_events.stop_times[i] > 500000) {
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
      draw_prompt(string_data(&line));
      line_changed = false;
    }

    // write to MARKLIN if we can (1ms delay)
    if (timer_value - timer_events.write > T_WRITE) {
      timer_events.write = timer_value;
      if (cbuf_len(&out_stream) > 0) {
        uint8_t byte = cbuf_front(&out_stream);
        if (uart_try_putc(MARKLIN, byte) == 0) cbuf_pop(&out_stream);
      }
    }

    uint64_t end_timer_value = timer_get();
    uint64_t main_loop_time = end_timer_value - timer_value;
    if (main_loop_time > worst_main_loop_time) {
      worst_main_loop_time = main_loop_time;
      uart_printf(CONSOLE, "\033[31;0Hworst main loop time: %u", worst_main_loop_time);
    }
    if (timer_value - timer_events.dev > 100000) {
      timer_events.dev = timer_value;
      uart_printf(CONSOLE, "\033[30;0H%smain loop time: %u", ANSI_CLEAR_LINE, main_loop_time);
    }

  }

  // U-Boot displays the return value from main - might be handy for debugging
  return 0;
}
