#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stddef.h>

void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_writestring(const char* data);
void terminal_setcolor(uint8_t color);

#endif
