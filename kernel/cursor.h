#ifndef CURSOR_H
#define CURSOR_H

#include <stdint.h>

void cursor_enable(uint8_t cursor_start, uint8_t cursor_end);
void cursor_disable(void);
void cursor_set_position(uint8_t x, uint8_t y);
uint16_t cursor_get_position(void);
void cursor_move_right(void);
void cursor_move_left(void);

#endif
