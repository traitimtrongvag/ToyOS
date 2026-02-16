#include "port.h"
#include <stdint.h>
#include <stdbool.h>

extern void terminal_putchar(char c);

#define KEYBOARD_DATA_PORT 0x60
#define KEY_RELEASE_MASK   0x80
#define SCANCODE_SHIFT_L   0x2A
#define SCANCODE_SHIFT_R   0x36
#define SCANCODE_CAPS_LOCK 0x3A

static const char scancode_table_normal[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_table_shifted[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

static bool shift_active = false;
static bool caps_lock_on = false;

static char apply_case(char c) {
    if (c < 'a' || c > 'z') return c;
    return (caps_lock_on ^ shift_active) ? (c - 32) : c;
}

static char resolve_char(uint8_t scancode) {
    if (scancode >= sizeof(scancode_table_normal)) return 0;

    if (shift_active) {
        char c = scancode_table_shifted[scancode];
        if (c >= 'A' && c <= 'Z') return apply_case(c - 32);
        return c;
    }

    return apply_case(scancode_table_normal[scancode]);
}

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    bool is_release = scancode & KEY_RELEASE_MASK;
    uint8_t key = scancode & ~KEY_RELEASE_MASK;

    if (key == SCANCODE_SHIFT_L || key == SCANCODE_SHIFT_R) {
        shift_active = !is_release;
        return;
    }

    if (key == SCANCODE_CAPS_LOCK && !is_release) {
        caps_lock_on = !caps_lock_on;
        return;
    }

    if (is_release) return;

    char c = resolve_char(key);
    if (c != 0) {
        extern void shell_handle_input(char);
        shell_handle_input(c);
    }
}

void keyboard_init(void) {
    shift_active = false;
    caps_lock_on = false;
}
