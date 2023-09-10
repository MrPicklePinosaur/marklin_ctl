#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdint.h>

typedef struct {
  enum {
    TRAIN_SPEED,
    REVERSE,
    SWITCH,
    QUIT,
  } _type;

  union {
    struct {
      uint32_t train;
      uint32_t speed;
    } train_speed;
    
    struct {
      uint32_t train;
    } reverse;

  } _data;
} ParserResult;

void parse_command(const char* command);

#endif // __PARSER_H__
