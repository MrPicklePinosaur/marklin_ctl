#include <ctype.h>

#include "rpi.h"
#include "util.h"
#include "marklin.h"
#include "parser.h"
#include "string.h"

#define NUMBER_OF_TRAINS 80

// Formats system time into human readable string
void fmt_time(uint64_t time) {
  
  unsigned int f_tenths = time % 1000000 / 10000;
  unsigned int secs = time / 1000000;
  unsigned int f_secs = secs % 60;
  unsigned int f_min = secs / 60;

  uart_printf(CONSOLE, "%sTIME %u:%u:%u ", ANSI_MOVE("0", "0"), f_min, f_secs, f_tenths);

}

// Keeps track of the last time each event was ran
typedef struct {
  // Last time the timer was updated
  uint32_t timer;
  // Last time the state of the sensors were dumped
  uint32_t sensor;
} TimerEvents;

TimerEvents
timerevents_new(void)
{
  TimerEvents timer_events = {
    .timer = 0,
    .sensor = 0,
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

  uint64_t timer_value = 0;

  String line = string_new();

  uart_printf(CONSOLE, "%s%s", ANSI_CLEAR, ANSI_ORIGIN);

  bool line_changed = true; // flag used to decide when to redraw the line
  uint32_t log_length = 0; // number of lines of log text we have printed out
  uint32_t sensor_bytes_expecting = 0; // number of bytes we are expecting to read from sensors
  while (1) {

    timer_value = timer_get();

    // should the timer be updated
    if (timer_value - timer_events.timer > 10000) {
      timer_events.timer = timer_value;
      fmt_time(timer_value);
    }

    // poll switches
    /* uart_printf(CONSOLE, "%s", ANSI_MOVE("1", "0")); */

    if (timer_value - timer_events.sensor > 10000 && sensor_bytes_expecting == 0) {
      timer_events.sensor = timer_value;
      marklin_dump_s88();
      sensor_bytes_expecting = 10; // 5 sensors with 2 bytes each
    }

    // check if terminal has a byte
    unsigned char c = 0;
    uart_getc_poll(CONSOLE, &c);

    // check if sensors sent a byte
    if (sensor_bytes_expecting > 0) {
      unsigned char sensor_byte = 0;
      // if we had data
      if (uart_getc_poll(MARKLIN, &sensor_byte)) {
        --sensor_bytes_expecting;
        uart_printf(CONSOLE, "\033[20;0H\033[K sensor bytes %u", sensor_bytes_expecting);
      }
    }

    /* uart_printf(CONSOLE, "\r\ngot character %d", c); */

    if (isalnum(c) || isblank(c)) {
      string_pushc(&line, c);
      line_changed = true;
    }
    else if (c == 0x0d) {
      // enter is pressed

      uart_printf(CONSOLE, "\033[%u;0H", 3 + log_length);

      // parse the line
      ParserResult parser_result = parse_command(string_data(&line));

      if (parser_result._type == PARSER_RESULT_TRAIN_SPEED) {
        uint32_t train = parser_result._data.train_speed.train;
        uint32_t speed = parser_result._data.train_speed.speed;
        marklin_train_ctl(train, speed);
        train_state[train] = speed;
        uart_printf(CONSOLE, "sending command for train %u at speed %u", train, speed);
        ++log_length;
      }
      else if (parser_result._type == PARSER_RESULT_REVERSE) {
        uint32_t train = parser_result._data.reverse.train;

        uint32_t cur_speed = train_state[train];

        marklin_train_ctl(train, SPEED_STOP);

        for (unsigned int i = 0; i < 10000000; ++i) {}

        marklin_train_ctl(train, SPEED_REVERSE);

        for (unsigned int i = 0; i < 10000000; ++i) {}

        marklin_train_ctl(train, cur_speed);

        uart_printf(CONSOLE, "reversing direction for train %u", train);
        ++log_length;
      }
      else if (parser_result._type == PARSER_RESULT_QUIT) {
        uart_printf(CONSOLE, "exiting... goodbye!");
        ++log_length;
        break;
      }
      else {
        uart_printf(CONSOLE, "invalid command");
        ++log_length;
      }

      string_clear(&line);
      line_changed = true;

    }
    else if (c == 0x08) {
      // backspace is pressed
      string_popc(&line);
      line_changed = true;
    }

    if (line_changed) {
      uart_printf(CONSOLE, "%s%s", ANSI_MOVE("2", "0"), ANSI_CLEAR_LINE);
      uart_printf(CONSOLE, "marklin> %s", string_data(&line));
      line_changed = false;
    }

  }

  // U-Boot displays the return value from main - might be handy for debugging
  return 0;
}
