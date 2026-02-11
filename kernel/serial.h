#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#define SERIAL_COM1 0x3F8
#define SERIAL_COM2 0x2F8
#define SERIAL_COM3 0x3E8
#define SERIAL_COM4 0x2E8

void serial_init(void);
void serial_putchar(char c);
void serial_write(const char* str);
void serial_write_hex(uint32_t value);
void serial_write_dec(uint32_t value);

#endif
