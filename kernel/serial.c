#include "serial.h"
#include "port.h"

#define SERIAL_REG_DATA         0
#define SERIAL_REG_INT_ENABLE   1
#define SERIAL_REG_DIVISOR_LO   0
#define SERIAL_REG_DIVISOR_HI   1
#define SERIAL_REG_LINE_CTRL    3
#define SERIAL_REG_MODEM_CTRL   4
#define SERIAL_REG_LINE_STATUS  5
#define SERIAL_REG_FIFO_CTRL    2

#define SERIAL_LINE_CTRL_DLAB   0x80
#define SERIAL_LINE_CTRL_8N1    0x03
#define SERIAL_FIFO_ENABLE      0xC7
#define SERIAL_MODEM_READY      0x0B

#define SERIAL_BAUD_DIVISOR     3

#define SERIAL_STATUS_TX_EMPTY  0x20
#define SERIAL_STATUS_RX_READY  0x01

void serial_init(void) {
    outb(SERIAL_COM1 + SERIAL_REG_INT_ENABLE, 0x00);
    outb(SERIAL_COM1 + SERIAL_REG_LINE_CTRL,  SERIAL_LINE_CTRL_DLAB);
    outb(SERIAL_COM1 + SERIAL_REG_DIVISOR_LO, SERIAL_BAUD_DIVISOR);
    outb(SERIAL_COM1 + SERIAL_REG_DIVISOR_HI, 0x00);
    outb(SERIAL_COM1 + SERIAL_REG_LINE_CTRL,  SERIAL_LINE_CTRL_8N1);
    outb(SERIAL_COM1 + SERIAL_REG_FIFO_CTRL,  SERIAL_FIFO_ENABLE);
    outb(SERIAL_COM1 + SERIAL_REG_MODEM_CTRL, SERIAL_MODEM_READY);
}

static int serial_tx_ready(void) {
    return inb(SERIAL_COM1 + SERIAL_REG_LINE_STATUS) & SERIAL_STATUS_TX_EMPTY;
}

int serial_data_ready(void) {
    return inb(SERIAL_COM1 + SERIAL_REG_LINE_STATUS) & SERIAL_STATUS_RX_READY;
}

char serial_getchar(void) {
    while (!serial_data_ready());
    return (char)inb(SERIAL_COM1 + SERIAL_REG_DATA);
}

void serial_putchar(char c) {
    if (c == '\n')
        serial_putchar('\r');
    while (!serial_tx_ready());
    outb(SERIAL_COM1 + SERIAL_REG_DATA, c);
}

void serial_write(const char *str) {
    while (*str)
        serial_putchar(*str++);
}

void serial_write_hex(uint32_t value) {
    static const char hex_digits[] = "0123456789ABCDEF";
    serial_write("0x");
    for (int shift = 28; shift >= 0; shift -= 4)
        serial_putchar(hex_digits[(value >> shift) & 0xF]);
}

void serial_write_dec(uint32_t value) {
    if (value == 0) {
        serial_putchar('0');
        return;
    }

    static const uint32_t powers[] = {
        1000000000u, 100000000u, 10000000u, 1000000u,
        100000u,     10000u,     1000u,     100u,
        10u,         1u
    };
    static const int NUM_POWERS = 10;
    int started = 0;
    for (int p = 0; p < NUM_POWERS; p++) {
        uint32_t power = powers[p];
        if (!started && value < power) continue;
        started = 1;
        int digit = 0;
        while (value >= power) { value -= power; digit++; }
        serial_putchar('0' + digit);
    }
}
