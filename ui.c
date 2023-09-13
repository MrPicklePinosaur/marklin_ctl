#include "ui.h"
#include "rpi.h"
#include "util.h"

// Get screen reading for drawing
void
setup_ui(void)
{
  uart_printf(CONSOLE, "%s%s%s", ANSI_CLEAR, ANSI_ORIGIN, ANSI_HIDE);
}

/// Draw the part of UI that will never be re-rendered
void
draw_static_ui(void)
{
  uart_printf(CONSOLE, "\r\n");
  uart_printf(CONSOLE, "     ~~~~ ____   |~~~~~~~~~~~~~|   |~~~~~~~~~~~~~|   |~~~~~~~~~~~~~|\r\n");
  uart_printf(CONSOLE, "    Y_,___|[]|   | MARKLIN CTL |   |  CS452 F23  |   | Daniel  Liu |\r\n");
  uart_printf(CONSOLE, "   {|_|_|_|PU|_,_|_____________|-,-|_____________|-,-|_____________|\r\n");
  uart_printf(CONSOLE, "  //oo---OO=OO     OOO     OOO       000     000       000     000  \r\n");
  uart_printf(CONSOLE, "\r\n");
  uart_printf(CONSOLE, "╭───────────────────────────────────────────────────────────────────────────────╮\r\n");
  uart_printf(CONSOLE, "│ ○ ○ ○                            MARKLIN CTL                                  │\r\n");
  uart_printf(CONSOLE, "├─[console]────────────────────────────────────────────────┬─[sensors]──────────┤\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          │                    │\r\n");
  uart_printf(CONSOLE, "│                                                          ├─[switches]─────────┤\r\n");
  uart_printf(CONSOLE, "│                                                          │ 01 .     12 .      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 02 .     13 .      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 03 .     14 .      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 04 .     15 .      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 05 .     16 .      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 06 .     17 .      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 07 .     18 .      │\r\n");
  uart_printf(CONSOLE, "│                                                          │ 08 .               │\r\n");
  uart_printf(CONSOLE, "│╭────────────────────────────────────────────────────────╮│ 09 .               │\r\n");
  uart_printf(CONSOLE, "││>                                                       ││ 10 .               │\r\n");
  uart_printf(CONSOLE, "│╰────────────────────────────────────────────────────────╯│ 11 .               │\r\n");
  uart_printf(CONSOLE, "╰──────────────────────────────────────────────────────────┴────────────────────╯\r\n");
}

// Formats system time into human readable string
void
draw_time(uint64_t time) {
  
  unsigned int f_tenths = time % 1000000 / 100000;
  unsigned int secs = time / 1000000;
  unsigned int f_secs = secs % 60;
  unsigned int f_min = secs / 60;

  uart_printf(CONSOLE, "\033[8;%uH", 71);

  if (f_min < 10) uart_printf(CONSOLE, "0%u:", f_min);
  else uart_printf(CONSOLE, "%u:", f_min);

  if (f_secs < 10) uart_printf(CONSOLE, "0%u:", f_secs);
  else uart_printf(CONSOLE, "%u:", f_secs);

  uart_printf(CONSOLE, "%u0", f_tenths);

}

static const char* PROMPT_ANCHOR = "\033[27;5H";

void
draw_prompt(const char* prompt)
{
  uart_printf(CONSOLE, "%s                                                      ", PROMPT_ANCHOR);
  uart_printf(CONSOLE, "%s%s", PROMPT_ANCHOR, prompt);
}

void
clear_command_window(void)
{
  for (unsigned int i = 0; i < CONSOLE_MAX_LINES; ++i) {
    uart_printf(CONSOLE, "\033[%u;2H                                                          ", 10 + i);
  }

}

void
clear_sensor_window(void)
{
  for (int i = 0; i < 7; ++i) {
    uart_printf(CONSOLE, "\033[%u;62H                   ", 10 + i);
  }
}
