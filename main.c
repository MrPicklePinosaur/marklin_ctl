#include "rpi.h"
#include "util.h"

// Serial line 1 on the RPi hat is used for the console
static const size_t CONSOLE = 1;

// Formats system time into human readable string
void fmt_time(uint64_t time) {
  
  unsigned int f_tenths = time % 1000000 / 10000;
  unsigned int secs = time / 1000000;
  unsigned int f_secs = secs % 60;
  unsigned int f_min = secs / 60;

  uart_printf(CONSOLE, "\r\nTIME %u:%u:%u> ", f_min, f_secs, f_tenths);

}

int kmain() {
  char hello[] = "=-=-=-=-= Hello world, this is iotest (" __TIME__ ") =-=-=-=-=\r\nPress 'q' to reboot\r\n";

  // initialize both console and marklin uarts
  uart_init();

  // not strictly necessary, since line 1 is configured during boot
  // but we'll configure the line anyways, so we know what state it is in
  uart_config_and_enable(CONSOLE, 115200);

  uart_puts(CONSOLE, hello);

  uint64_t timer_value = 0;

  char c = 0;
  while (1) {

    // clear screen
    uart_printf(CONSOLE, "%s", ANSI_CLEAR);
    uart_printf(CONSOLE, "%s", ANSI_ORIGIN);

    timer_value = timer_get();

    uart_printf(CONSOLE, "\r\nPI[%u]> ", timer_value);
    fmt_time(timer_value);

    c = uart_getc_poll(CONSOLE);
    if (c == 'q') break;

  }
  uart_puts(CONSOLE, "\r\n");

  // U-Boot displays the return value from main - might be handy for debugging
  return 0;
}
