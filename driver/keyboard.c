#include "port.h"
#include <stdint.h>

extern void terminal_putchar(char c);

static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);
    
    if(scancode & 0x80) {
        return;
    }
    
    if(scancode < sizeof(scancode_to_ascii)) {
        char c = scancode_to_ascii[scancode];
        if(c != 0) {
            terminal_putchar(c);
        }
    }
}

void keyboard_init(void) {
}
