#include "speaker.h"
#include "../kernel/port.h"
#include "../kernel/timer.h"

#define PIT_FREQUENCY     1193180
#define PIT_CHANNEL2_PORT 0x42
/* PIT command: channel 2, lobyte/hibyte, square wave */
#define PIT_CMD_CHANNEL2  0xB6
#define SPEAKER_PORT      0x61
#define SPEAKER_ENABLE    0x03
#define SPEAKER_DISABLE   0xFC

static void pit_set_channel2(uint32_t frequency) {
    uint32_t divisor = PIT_FREQUENCY / frequency;
    outb(0x43, PIT_CMD_CHANNEL2);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

void speaker_init(void) {
    speaker_stop();
}

void speaker_play(uint32_t frequency) {
    if (frequency == 0) return;
    pit_set_channel2(frequency);
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state | SPEAKER_ENABLE);
}

void speaker_stop(void) {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state & SPEAKER_DISABLE);
}

void speaker_beep(uint32_t frequency, uint32_t duration_ms) {
    /* duration_ms converted to PIT ticks (timer runs at configurable Hz) */
    uint32_t ticks = duration_ms / 10;
    speaker_play(frequency);
    timer_wait(ticks);
    speaker_stop();
}
