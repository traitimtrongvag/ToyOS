#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define PIT_FREQUENCY 1193180
#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40

void timer_init(uint32_t frequency);
void timer_callback(void);
uint32_t timer_get_ticks(void);
void timer_wait(uint32_t ticks);

#endif
