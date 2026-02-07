#include "port.h"
#include <stdint.h>

#define SERIAL_COM1 0x3F8

void serial_init(void) {
    outb(SERIAL_COM1 + 1, 0x00);
    outb(SERIAL_COM1 + 3, 0x80);
    outb(SERIAL_COM1 + 0, 0x03);
    outb(SERIAL_COM1 + 1, 0x00);
    outb(SERIAL_COM1 + 3, 0x03);
    outb(SERIAL_COM1 + 2, 0xC7);
    outb(SERIAL_COM1 + 4, 0x0B);
}

static int serial_transmit_empty(void) {
    return inb(SERIAL_COM1 + 5) & 0x20;
}

void serial_putchar(char c) {
    while(serial_transmit_empty() == 0);
    outb(SERIAL_COM1, c);
}

void serial_write(const char* str) {
    while(*str) {
        serial_putchar(*str++);
    }
}

void serial_write_hex(uint32_t value) {
    const char hex[] = "0123456789ABCDEF";
    serial_write("0x");
    
    for(int i = 7; i >= 0; i--) {
        serial_putchar(hex[(value >> (i * 4)) & 0xF]);
    }
}

void serial_write_dec(uint32_t value) {
    if(value == 0) {
        serial_putchar('0');
        return;
    }
    
    char buffer[10];
    int i = 0;
    
    while(value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    while(i > 0) {
        serial_putchar(buffer[--i]);
    }
}
