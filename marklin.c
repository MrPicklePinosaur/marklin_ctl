#include "marklin.h"
#include "rpi.h"

// Read the state of all the detectors on the track
/*
uint32_t s88_get(void) {

}
*/

void marklin_train_ctl(uint32_t train, uint32_t speed) {
  /* const char cmd[2] = {(char)speed, (char)train}; */
  /* uart_puts(MARKLIN, cmd); */
  uart_putc(MARKLIN, (unsigned char)speed);
  uart_putc(MARKLIN, (unsigned char)train);

  // TODO temp delay
  for (unsigned int i = 0; i < 500; ++i) {}
}

void marklin_dump_s88(void) {
  // dump all 5 s88 decoders
  /* uart_putc(MARKLIN, (unsigned char)128+5); */

  uart_putc(MARKLIN, (unsigned char)192+1);
  unsigned char left = uart_getc(MARKLIN);
  unsigned char right = uart_getc(MARKLIN);

  uart_printf(CONSOLE, "\r\npolled %u %u", left, right);

  for (unsigned int i = 0; i < 500; ++i) {}

}
