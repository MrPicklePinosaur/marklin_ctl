#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdint.h>

typedef struct {
  enum {
    PARSER_RESULT_TRAIN_SPEED,
    PARSER_RESULT_REVERSE,
    PARSER_RESULT_SWITCH,
    PARSER_RESULT_QUIT,
    PARSER_RESULT_ERROR,
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

ParserResult parse_command(const char* command);

#endif // __PARSER_H__
