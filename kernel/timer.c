#include "port.h"
#include <stdint.h>

extern void terminal_writestring(const char* s);

static uint32_t tick_count = 0;

void timer_callback(void) {
    tick_count++;
}

void timer_phase(uint32_t hz) {
    uint32_t divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

void timer_init(uint32_t frequency) {
    timer_phase(frequency);
}

uint32_t timer_get_ticks(void) {
    return tick_count;
}

void timer_wait(uint32_t ticks) {
    uint32_t target = tick_count + ticks;
    while(tick_count < target) {
        __asm__ volatile("hlt");
    }
}
