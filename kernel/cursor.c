#include <stdint.h>
#include "port.h"

#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5
#define VGA_WIDTH 80

void cursor_enable(uint8_t cursor_start, uint8_t cursor_end) {
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) & 0xC0) | cursor_start);
    outb(VGA_CTRL_REGISTER, 0x0B);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) & 0xE0) | cursor_end);
}

void cursor_disable(void) {
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, 0x20);
}

void cursor_set_position(uint8_t x, uint8_t y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(VGA_CTRL_REGISTER, 15);
    outb(VGA_DATA_REGISTER, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL_REGISTER, 14);
    outb(VGA_DATA_REGISTER, (uint8_t)((pos >> 8) & 0xFF));
}

uint16_t cursor_get_position(void) {
    uint16_t pos = 0;
    outb(VGA_CTRL_REGISTER, 15);
    pos |= inb(VGA_DATA_REGISTER);
    outb(VGA_CTRL_REGISTER, 14);
    pos |= ((uint16_t)inb(VGA_DATA_REGISTER)) << 8;
    return pos;
}

void cursor_move_right(void) {
    uint16_t pos = cursor_get_position();
    if ((pos % VGA_WIDTH) < VGA_WIDTH - 1) {
        cursor_set_position((pos % VGA_WIDTH) + 1, pos / VGA_WIDTH);
    }
}

void cursor_move_left(void) {
    uint16_t pos = cursor_get_position();
    if ((pos % VGA_WIDTH) > 0) {
        cursor_set_position((pos % VGA_WIDTH) - 1, pos / VGA_WIDTH);
    }
}
