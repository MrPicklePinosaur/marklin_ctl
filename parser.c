#include <ctype.h>
#include <string.h>

#include "parser.h"
#include "string.h"

#include "rpi.h"

String get_word(const char* command, uint32_t* it);
void eat_whitespace(const char* command, uint32_t* it);
int get_number(const char* command, uint32_t* it);

ParserResult
parse_command(const char* command)
{
  // read until first whitespace character
  uint32_t it = 0;

  String cmd_name = get_word(command, &it);

  /* uart_printf(CONSOLE, "\r\nread %s", string_data(&cmd_name)); */

  if (strcmp(string_data(&cmd_name), "tr") == 0) {

    eat_whitespace(command, &it);

    int train = get_number(command, &it);

    eat_whitespace(command, &it);

    int speed = get_number(command, &it);

    /* uart_printf(CONSOLE, "\r\ngot tr command train = %d, speed = %d", train, speed); */

    ParserResult res = {
      ._type = PARSER_RESULT_TRAIN_SPEED,
      ._data = {
        .train_speed = {
          .train = train,
          .speed = speed,
        }
      }
    };
    return res;

  }
  else if (strcmp(string_data(&cmd_name), "rv") == 0) {

    eat_whitespace(command, &it);

    int train = get_number(command, &it);

    /* uart_printf(CONSOLE, "\r\ngot rv command train = %d", train); */

    ParserResult res = {
      ._type = PARSER_RESULT_REVERSE,
      ._data = {
        .reverse = {
          .train = train,
        }
      }
    };
    return res;
  }
  else if (strcmp(string_data(&cmd_name), "q") == 0) {

    /* uart_printf(CONSOLE, "\r\ngot quit command"); */

    ParserResult res = {
      ._type = PARSER_RESULT_QUIT,
      ._data = {}
    };
    return res;
  }

  ParserResult res = {
    ._type = PARSER_RESULT_ERROR,
    ._data = {},
  };
  return res;

}

String
get_word(const char* command, uint32_t* it)
{
  String word = string_new();
  while (1) {
    char c = command[*it]; 
    if (c == 0) break;

    if (isalnum(c)) {
      string_pushc(&word, c);
    }
    else if (isspace(c)) {
      break;
    }
    
    ++(*it);
  }
  return word;
}

int
get_number(const char* command, uint32_t* it)
{
  int number = 0;
  while (1) {
    char c = command[*it]; 
    if (c == 0) break;

    if (! isdigit(c)) break;

    number = number*10 + (c-'0');
    
    ++(*it);
  }
  return number;
}

void
eat_whitespace(const char* command, uint32_t* it)
{
  while (1) {
    char c = command[*it]; 
    if (c == 0) break;

    if (!isspace(c)) break;
    
    ++(*it);
  }
}
