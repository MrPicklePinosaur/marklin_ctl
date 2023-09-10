#include "marklin.h"
#include "rpi.h"

static const uint8_t SPEED_STOP     = 0x0;
static const uint8_t SPEED_REVERSE  = 0xF;

static const size_t MARKLIN = 2;

// Read the state of all the detectors on the track
/*
uint32_t s88_get(void) {

}
*/

void marklin_train_ctl(uint32_t train, uint32_t speed) {
  /* const char cmd[2] = {(char)speed, (char)train}; */
  /* uart_puts(MARKLIN, cmd); */
  uart_putc(MARKLIN, (unsigned char)speed);
  uart_putc(MARKLIN, (unsigned char)train);

  // TODO temp delay
  for (unsigned int i = 0; i < 500; ++i) {}
}


