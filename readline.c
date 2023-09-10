#include "readline.h"

Readline
readline_new(void)
{
  Readline readline = {
    .data = {0},
    .length = 0,
  };

  return readline;
}

int
readline_pushc(Readline* readline, char c)
{
  if (readline->length >= MAX_LINE_LENGTH) return -1;

  readline->data[readline->length] = c;
  readline->data[readline->length+1] = 0;
  ++(readline->length);

  return 0;
}

const char*
readline_data(Readline* readline)
{
  return (const char*)readline->data;
}

uint32_t
readline_len(Readline* readline)
{
  return readline->length;
}

void
readline_clear(Readline* readline)
{
  readline->length = 0;
  readline->data[0] = 0;
}

