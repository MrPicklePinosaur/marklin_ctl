#include <ctype.h>

#include "parser.h"
#include "string.h"

void
parse_command(const char* command)
{
  // read until first whitespace character
  uint32_t it = 0;

  String cmd_name = string_new();
  while (1) {
    char c = command[it]; 
    if (c == 0) break;

    if (isalnum(c)) {
      string_pushc(&cmd_name, c);
    }
    else if (isspace(c)) {
      break;
    }
    
    ++it;
  }


}
