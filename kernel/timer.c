#include "timer.h"
#include "port.h"

static uint32_t tick_count = 0;
uint32_t timer_ticks = 0;

void timer_handler(void) {
    tick_count++;
    timer_ticks = tick_count;
}

void timer_callback(void) {
    timer_handler();
}

static void timer_set_phase(uint32_t frequency) {
    uint32_t divisor = PIT_FREQUENCY / frequency;
    uint8_t low_byte = divisor & 0xFF;
    uint8_t high_byte = (divisor >> 8) & 0xFF;
    
    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, low_byte);
    outb(PIT_CHANNEL0_PORT, high_byte);
}

void timer_init(uint32_t frequency) {
    tick_count = 0;
    timer_set_phase(frequency);
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

void timer_install(void) {
    timer_init(100);
}

