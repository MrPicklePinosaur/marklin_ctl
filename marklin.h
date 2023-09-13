#ifndef __MARKLIN_H__
#define __MARKLIN_H__

#include <stdint.h>
#include <stdbool.h>
#include "cbuf.h"

static const uint8_t SPEED_STOP     = 0x0;
static const uint8_t SPEED_REVERSE  = 0xF;

typedef enum {
  SWITCH_GROUP_A = 0,
  SWITCH_GROUP_B,
  SWITCH_GROUP_C,
  SWITCH_GROUP_D,
  SWITCH_GROUP_E,
} SwitchGroup;

typedef enum {
  SWITCH_MODE_STRAIGHT = 33,
  SWITCH_MODE_CURVED = 34,
} SwitchMode;

void marklin_train_ctl(CBuf* out_stream, uint32_t train, uint32_t speed);
void marklin_switch_ctl(CBuf* out_stream, uint32_t switch_id, SwitchMode mode);
void marklin_dump_s88(CBuf* out_stream);
void marklin_go(CBuf* out_stream);
void marklin_stop(CBuf* out_stream);

typedef struct {
  uint8_t _data[10];
  // data from last update (used to track when state of switches has changed)
  uint8_t _prev_data[10];
} SwitchTable;

SwitchTable switchtable_new(void);
/* bool switchtable_test(SwitchTable* switch_table, SwitchGroup group, uint8_t num); */
uint8_t switchtable_write(SwitchTable* switch_table, uint32_t index, uint8_t data);

#endif // __MARKLIN_H__
