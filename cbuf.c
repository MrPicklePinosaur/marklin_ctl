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

// Returns 1 if buffer is full
int
cbuf_push(CBuf* cbuf, uint8_t byte)
{
  if (cbuf->_len >= CBUF_MAX_LENGTH) return 1;

  cbuf->_back_ptr = (cbuf->_back_ptr+1) % CBUF_MAX_LENGTH;
  cbuf->_data[cbuf->_back_ptr] = byte;

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
