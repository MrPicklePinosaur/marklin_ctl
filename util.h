#ifndef _util_h_
#define _util_h_ 1

#include <stddef.h>

// ansi codes
#define ANSI_CLEAR "\033[2J"
#define ANSI_HIDE "\033[?25l"
#define ANSI_ORIGIN "\033[H"
#define ANSI_MOVE(r, c) "\033[" r ";" c "H"
#define ANSI_CLEAR_LINE "\033[K"

// conversions
int a2d(char ch);
char a2i( char ch, char **src, int base, int *nump );
void ui2a( unsigned int num, unsigned int base, char *bf );
void i2a( int num, char *bf );

// memory
void *memset(void *s, int c, size_t n);
void* memcpy(void* restrict dest, const void* restrict src, size_t n);

#endif /* util.h */
