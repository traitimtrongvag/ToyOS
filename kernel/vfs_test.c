#include "vfs.h"
#include "terminal.h"
#include "string.h"

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
        (const uint8_t*)filename,
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
        (const uint8_t*)filename,
        strlen(filename),
        (const uint8_t*)content,
        strlen(content)
    );

    if (written > 0) {
        terminal_writestring("[VFS] Wrote ");
        char num_buf[16];
        itoa(written, num_buf, 10);
        terminal_writestring(num_buf);
        terminal_writestring(" bytes\n");
    } else {
        terminal_writestring("[VFS] Write failed\n");
        return;
    }

    terminal_writestring("[VFS] Reading file...\n");

    memset(read_buffer, 0, sizeof(read_buffer));
    int32_t read_bytes = rust_vfs_read(
        (const uint8_t*)filename,
        strlen(filename),
        (uint8_t*)read_buffer,
        sizeof(read_buffer) - 1
    );

    if (read_bytes > 0) {
        terminal_writestring("[VFS] Read ");
        char num_buf[16];
        itoa(read_bytes, num_buf, 10);
        terminal_writestring(num_buf);
        terminal_writestring(" bytes: \"");
        terminal_writestring(read_buffer);
        terminal_writestring("\"\n");
        terminal_writestring("[VFS] Test passed!\n");
    } else {
        terminal_writestring("[VFS] Read failed\n");
    }
}

void itoa(int32_t value, char* str, int base) {
    char* p = str;
    char* p1, *p2;
    uint32_t ud = value;
    int32_t divisor = 10;

    if (base == 16) divisor = 16;

    do {
        int32_t remainder = ud % divisor;
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (ud /= divisor);

    *p = 0;

    p1 = str;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}
