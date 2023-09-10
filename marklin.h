#ifndef __MARKLIN_H__
#define __MARKLIN_H__

#include <stdint.h>

static const uint8_t SPEED_STOP     = 0x0;
static const uint8_t SPEED_REVERSE  = 0xF;

void marklin_train_ctl(uint32_t train, uint32_t speed);

#endif // __MARKLIN_H__
