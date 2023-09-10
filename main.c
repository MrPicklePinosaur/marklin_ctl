#include <ctype.h>

#include "rpi.h"
#include "util.h"
#include "marklin.h"
#include "readline.h"

// Serial line 1 on the RPi hat is used for the console
static const size_t CONSOLE = 1;

// Serial line 2 is the train control
static const size_t MARKLIN = 2;

// Formats system time into human readable string
void fmt_time(uint64_t time) {
  
  unsigned int f_tenths = time % 1000000 / 10000;
  unsigned int secs = time / 1000000;
  unsigned int f_secs = secs % 60;
  unsigned int f_min = secs / 60;

  uart_printf(CONSOLE, "\r\nTIME %u:%u:%u> ", f_min, f_secs, f_tenths);

}


int kmain() {

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

  uint64_t timer_value = 0;

  Readline readline = readline_new();

  char c = 0;
  while (1) {

    // clear screen
    /* uart_printf(CONSOLE, "%s%s", ANSI_CLEAR, ANSI_ORIGIN); */

    timer_value = timer_get();

    fmt_time(timer_value);

    c = uart_getc(CONSOLE);
    /* c = uart_getc_poll(CONSOLE); */

    /* if (c == 'q') break; */
    /* else if (c == 's') { */
    /*   uint32_t train = 1; */
    /*   uint32_t speed = 1; */
    /*   marklin_train_ctl(train, speed); */
    /*   uart_printf(CONSOLE, "\r\ntrain %u at speed %u", train, speed); */
    /* } */

    uart_printf(CONSOLE, "\r\ngot character %d", c);

    if (isalnum(c)) {
      readline_pushc(&readline, c);
    }
    else if (c == 0x0d) {
      // enter is pressed
      readline_clear(&readline);

      break;
    }
    else if (c == 0x08) {
      // backspace is pressed

    }

    uart_printf(CONSOLE, "\r\n%s", readline_data(&readline));

    // waste some time
    for (unsigned int i = 0; i < 1000; ++i) {}

  }

  // U-Boot displays the return value from main - might be handy for debugging
  return 0;
}
