#ifndef __READLINE_H__
#define __READLINE_H__

#include <stdint.h>

#define MAX_LINE_LENGTH 128

// TODO a bit hard to do opaque types without being able to use malloc to create a Readline*
typedef struct Readline {
  // Always null terminated
  char data[MAX_LINE_LENGTH+1];
  uint32_t length;
} Readline;

Readline readline_new(void);
int readline_pushc(Readline* readline, char c);
const char* readline_data(Readline* readline);
uint32_t readline_len(Readline* readline);
void readline_clear(Readline* readline);

#endif // __READLINE_H__
