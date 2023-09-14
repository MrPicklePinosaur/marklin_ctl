#ifndef __UI_H__
#define __UI_H__

#include "stdint.h"

static const unsigned int CONSOLE_MAX_LINES = 16;
static const unsigned int SENSOR_LOG_MAX_ENTRIES = 35;
                                                   
void setup_ui(void);
void draw_static_ui(void);
void draw_time(uint64_t time);
void draw_prompt(const char* prompt);
void draw_switch(uint32_t switch_id, char mode);
void clear_command_window(void);
void clear_sensor_window(void);

#endif // __UI_H__
