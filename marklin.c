#include "marklin.h"
#include "rpi.h"

void marklin_train_ctl(CBuf* out_stream, uint32_t train, uint32_t speed) {
  cbuf_push(out_stream, speed);
  cbuf_push(out_stream, train);
}

void marklin_switch_ctl(CBuf* out_stream, uint32_t switch_id, SwitchMode mode) {
  cbuf_push(out_stream, mode);
  cbuf_push(out_stream, switch_id);
  cbuf_push(out_stream, 32); // reset command
}

void marklin_stop(CBuf* out_stream) {
  cbuf_push(out_stream, 97);
  cbuf_push(out_stream, 97);
}

void marklin_go(CBuf* out_stream) {
  cbuf_push(out_stream, 96);
  cbuf_push(out_stream, 96);
}

void marklin_dump_s88(CBuf* out_stream) {
  cbuf_push(out_stream, 128+5);
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

