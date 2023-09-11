#ifndef __MARKLIN_H__
#define __MARKLIN_H__

#include <stdint.h>
#include <stdbool.h>

static const uint8_t SPEED_STOP     = 0x0;
static const uint8_t SPEED_REVERSE  = 0xF;

void marklin_train_ctl(uint32_t train, uint32_t speed);
void marklin_dump_s88(void);

typedef enum {
  SWITCH_GROUP_A = 0,
  SWITCH_GROUP_B,
  SWITCH_GROUP_C,
  SWITCH_GROUP_D,
  SWITCH_GROUP_E,
} SwitchGroup;

typedef struct {
  uint16_t _data[5];
  // data from last update (used to track when state of switches has changed)
  uint16_t _prev_data[5];
} SwitchTable;

SwitchTable switchtable_new(void);
bool switchtable_test(SwitchTable* switch_table, SwitchGroup group, uint8_t num);
void switchtable_write(SwitchTable* switch_table, SwitchGroup group, uint16_t data);

#endif // __MARKLIN_H__
