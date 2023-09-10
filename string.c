#include "string.h"

String
string_new(void)
{
  String string = {
    .data = {0},
    .length = 0,
  };

  return string;
}

String
string_from_cstr(const char* cstr)
{
  String new_str = string_new();

  // TODO not efficient lol
  unsigned int i = 0;
  while (cstr[i] != 0) string_pushc(&new_str, cstr[i]);

  return new_str;
}

int
string_pushc(String* string, char c)
{
  if (string->length >= MAX_LINE_LENGTH) return -1;

  string->data[string->length] = c;
  string->data[string->length+1] = 0;
  ++(string->length);

  return 0;
}

void
string_popc(String* string)
{
  if (string->length == 0) return;

  --(string->length);
  string->data[string->length] = 0;
}

const char*
string_data(String* string)
{
  return (const char*)string->data;
}

uint32_t
string_len(String* string)
{
  return string->length;
}

void
string_clear(String* string)
{
  string->length = 0;
  string->data[0] = 0;
}

bool
string_cmp(String* a, String* b)
{
  if (string_len(a) != string_len(b)) return false;

  for (unsigned int i = 0; i < string_len(a); ++i)
    if (a->data[i] != b->data[i]) return false;

  return true;
}
