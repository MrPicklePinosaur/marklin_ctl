#ifndef __CBUF_H__
#define __CBUF_H__

#include <stdint.h>

/* circular buffer implementation */

#define CBUF_MAX_LENGTH 64

typedef struct {
  uint8_t _data[CBUF_MAX_LENGTH];
  uint32_t _front_ptr;
  uint32_t _back_ptr;
  uint32_t _len;
} CBuf;

CBuf cbuf_new(void);
int cbuf_push(CBuf* cbuf, uint8_t byte);
uint8_t cbuf_pop(CBuf* cbuf);
uint32_t cbuf_len(CBuf* cbuf);

#endif // __CBUF_H__
