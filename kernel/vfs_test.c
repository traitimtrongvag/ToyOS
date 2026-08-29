#include "vfs.h"
#include "terminal.h"
#include "string.h"

/* Subtraction-based itoa to avoid div instructions (baremetal compat). */
static void itoa_sub(int32_t value, char* buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    static const uint32_t powers[] = {
        1000000000u, 100000000u, 10000000u, 1000000u,
        100000u,     10000u,     1000u,     100u,
        10u,         1u
    };
    static const int NUM_POWERS = 10;
    int i = 0;
    uint32_t uval;
    if (value < 0) { buf[i++] = '-'; uval = (uint32_t)(-(value + 1)) + 1u; }
    else           { uval = (uint32_t)value; }
    int started = 0;
    for (int p = 0; p < NUM_POWERS; p++) {
        uint32_t power = powers[p];
        if (!started && uval < power) continue;
        started = 1;
        int digit = 0;
        while (uval >= power) { uval -= power; digit++; }
        buf[i++] = '0' + digit;
    }
    buf[i] = '\0';
}

void vfs_demo(void) {
    terminal_writestring("\n[VFS] Initializing virtual file system...\n");
    rust_vfs_init();

    const char* filename = "hello.txt";
    const char* content = "Hello from ToyOS VFS!";
    char read_buffer[64];

    terminal_writestring("[VFS] Creating file: ");
    terminal_writestring(filename);
    terminal_writestring("\n");

    int32_t result = rust_vfs_create(
        (const char*)filename,
        strlen(filename),
        VFS_TYPE_REGULAR
    );

    if (result == 0) {
        terminal_writestring("[VFS] File created successfully\n");
    } else {
        terminal_writestring("[VFS] Failed to create file\n");
        return;
    }

    terminal_writestring("[VFS] Writing data: \"");
    terminal_writestring(content);
    terminal_writestring("\"\n");

    int32_t written = rust_vfs_write(
        (const char*)filename,
        strlen(filename),
        (const char*)content,
        strlen(content)
    );

    if (written > 0) {
        terminal_writestring("[VFS] Wrote ");
        char num_buf[16];
        itoa_sub(written, num_buf);
        terminal_writestring(num_buf);
        terminal_writestring(" bytes\n");
    } else {
        terminal_writestring("[VFS] Write failed\n");
        return;
    }

    terminal_writestring("[VFS] Reading file...\n");

    memset(read_buffer, 0, sizeof(read_buffer));
    int32_t read_bytes = rust_vfs_read(
        (const char*)filename,
        strlen(filename),
        (uint8_t*)read_buffer,
        sizeof(read_buffer) - 1
    );

    if (read_bytes > 0) {
        terminal_writestring("[VFS] Read ");
        char num_buf[16];
        itoa_sub(read_bytes, num_buf);
        terminal_writestring(num_buf);
        terminal_writestring(" bytes: \"");
        terminal_writestring(read_buffer);
        terminal_writestring("\"\n");
        terminal_writestring("[VFS] Test passed!\n");
    } else {
        terminal_writestring("[VFS] Read failed\n");
    }
}