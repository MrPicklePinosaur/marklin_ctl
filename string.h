#ifndef __STRING_H__
#define __STRING_H__

#include <stdint.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 128

// TODO a bit hard to do opaque types without being able to use malloc to create a String*
typedef struct String {
  // Always null terminated
  char data[MAX_LINE_LENGTH+1];
  uint32_t length;
} String;

String string_new(void);
String string_from_cstr(const char* cstr);
int string_pushc(String* string, char c);
void string_popc(String* string);
const char* string_data(String* string);
uint32_t string_len(String* string);
void string_clear(String* string);
bool string_cmp(String* a, String* b);

#endif // __STRING_H__
