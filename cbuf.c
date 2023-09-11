#include "cbuf.h"

CBuf
cbuf_new(void)
{
  CBuf cbuf = {
    ._data = {0},
    ._front_ptr = 0,
    ._back_ptr = 0,
    ._len = 0,
  };
  return cbuf;
}

uint8_t
cbuf_front(CBuf* cbuf)
{
   return cbuf->_data[cbuf->_front_ptr];
}

uint8_t
cbuf_back(CBuf* cbuf)
{
  // TODO handle if empty
  return cbuf->_data[(cbuf->_back_ptr-1) % CBUF_MAX_LENGTH];
}

// Returns 1 if buffer is full
int
cbuf_push(CBuf* cbuf, uint8_t byte)
{
  if (cbuf->_len >= CBUF_MAX_LENGTH) return 1;

  cbuf->_data[cbuf->_back_ptr] = byte;
  cbuf->_back_ptr = (cbuf->_back_ptr+1) % CBUF_MAX_LENGTH;

  ++(cbuf->_len);
  return 0;
}

uint8_t
cbuf_pop(CBuf* cbuf)
{
  // TODO handle if buf is empty
  uint8_t data = cbuf->_data[cbuf->_front_ptr];

  cbuf->_front_ptr = (cbuf->_front_ptr+1) % CBUF_MAX_LENGTH;
  --(cbuf->_len);

  return data;
}

uint32_t
cbuf_len(CBuf* cbuf)
{
  return cbuf->_len;
}

/* tests

  CBuf out_stream = cbuf_new();
  uart_printf(CONSOLE, "%u\r\n", cbuf_len(&out_stream));
  cbuf_push(&out_stream, 0x1);
  uart_printf(CONSOLE, "%u\r\n", cbuf_len(&out_stream));
  cbuf_push(&out_stream, 0x2);
  cbuf_push(&out_stream, 0x3);
  uart_printf(CONSOLE, "%u\r\n", cbuf_len(&out_stream));
  uint8_t val = cbuf_pop(&out_stream);
  uart_printf(CONSOLE, "%u, val = %u\r\n", cbuf_len(&out_stream), val);

*/
