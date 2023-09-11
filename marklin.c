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
  uart_putc(MARKLIN, (unsigned char)128+5);

  /* uart_putc(MARKLIN, (unsigned char)192+1); */

  for (unsigned int i = 0; i < 500; ++i) {}

  /* unsigned char left = uart_getc(MARKLIN); */
  /* unsigned char right = uart_getc(MARKLIN); */

  /* uart_printf(CONSOLE, "\033[20;0H\033[K polled %u %u", left, right); */

}

SwitchTable
switchtable_new(void)
{
  SwitchTable switch_table = {
    ._data = {0},
    ._prev_data = {0},
  };
  return switch_table;
}

/* bool */
/* switchtable_test(SwitchTable* switch_table, SwitchGroup group, uint8_t num) */
/* { */
/*   // TODO double check that num is [1,16] */
/*   return ((switch_table->_data[group-1]) >> num) == 0x1; */
/* } */

// Update the data for a single group (block of 8 sensors), also returns the newly triggered sensors
uint8_t
switchtable_write(SwitchTable* switch_table, uint32_t index, uint8_t data)
{
  switch_table->_prev_data[index] = switch_table->_data[index];
  switch_table->_data[index] = data;

  return ~(switch_table->_prev_data[index]) & switch_table->_data[index];
}

