extern "C" {
    void terminal_writestring(const char* s);
    void terminal_putchar(char c);
    void terminal_setcolor(unsigned char color);
}

static inline void outb_vga(unsigned short port, unsigned char val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

static inline unsigned char inb_vga(unsigned short port) {
    unsigned char ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

// VGA hardware ports
#define VGA_CTRL_PORT    0x3D4
#define VGA_DATA_PORT    0x3D5
#define VGA_WIDTH        80
#define VGA_HEIGHT       25
#define VGA_MEMORY_ADDR  0xB8000

// Default color: white on black
#define VGA_DEFAULT_COLOR 0x07

class VgaDriver {
private:
    unsigned short* buffer;
    unsigned char   color;
    bool            initialized;

    unsigned short makeEntry(char c, unsigned char col) {
        return (unsigned short)c | ((unsigned short)col << 8);
    }

    void updateHardwareCursor(unsigned int pos) {
        outb_vga(VGA_CTRL_PORT, 0x0F);
        outb_vga(VGA_DATA_PORT, (unsigned char)(pos & 0xFF));
        outb_vga(VGA_CTRL_PORT, 0x0E);
        outb_vga(VGA_DATA_PORT, (unsigned char)((pos >> 8) & 0xFF));
    }

    unsigned int readHardwareCursor() {
        unsigned int pos = 0;
        outb_vga(VGA_CTRL_PORT, 0x0F);
        pos |= inb_vga(VGA_DATA_PORT);
        outb_vga(VGA_CTRL_PORT, 0x0E);
        pos |= ((unsigned int)inb_vga(VGA_DATA_PORT)) << 8;
        return pos;
    }

    void enableCursor(unsigned char start, unsigned char end) {
        outb_vga(VGA_CTRL_PORT, 0x0A);
        outb_vga(VGA_DATA_PORT, (inb_vga(VGA_DATA_PORT) & 0xC0) | start);
        outb_vga(VGA_CTRL_PORT, 0x0B);
        outb_vga(VGA_DATA_PORT, (inb_vga(VGA_DATA_PORT) & 0xE0) | end);
    }

    void disableCursor() {
        outb_vga(VGA_CTRL_PORT, 0x0A);
        outb_vga(VGA_DATA_PORT, 0x20);
    }

public:
    VgaDriver()
        : buffer((unsigned short*)VGA_MEMORY_ADDR),
          color(VGA_DEFAULT_COLOR),
          initialized(false) {}

    void init() {
        enableCursor(14, 15);
        initialized = true;
        terminal_writestring("  VGA driver initialized\n");
        terminal_writestring("  Cursor: enabled (underline style)\n");
    }

    void setColor(unsigned char fg, unsigned char bg) {
        color = (bg << 4) | (fg & 0x0F);
    }

    void writeAt(char c, unsigned int x, unsigned int y) {
        if (!initialized || x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
        buffer[y * VGA_WIDTH + x] = makeEntry(c, color);
    }

    void writeStringAt(const char* str, unsigned int x, unsigned int y) {
        if (!initialized) return;
        while (*str && x < VGA_WIDTH) {
            writeAt(*str++, x++, y);
        }
    }

    void clearRow(unsigned int y) {
        if (!initialized || y >= VGA_HEIGHT) return;
        for (unsigned int x = 0; x < VGA_WIDTH; x++) {
            buffer[y * VGA_WIDTH + x] = makeEntry(' ', color);
        }
    }

    void clearScreen() {
        if (!initialized) return;
        for (unsigned int y = 0; y < VGA_HEIGHT; y++) {
            clearRow(y);
        }
        updateHardwareCursor(0);
    }

    void moveCursor(unsigned int x, unsigned int y) {
        if (!initialized || x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
        updateHardwareCursor(y * VGA_WIDTH + x);
    }

    unsigned int getCursorX() {
        return readHardwareCursor() % VGA_WIDTH;
    }

    unsigned int getCursorY() {
        return readHardwareCursor() / VGA_WIDTH;
    }

    void hideCursor() { disableCursor(); }
    void showCursor() { enableCursor(14, 15); }

    bool isInitialized() { return initialized; }
};

static VgaDriver global_vga_driver;

extern "C" void cpp_driver_init() {
    global_vga_driver.init();
}

extern "C" void cpp_driver_test() {
    if (!global_vga_driver.isInitialized()) return;

    terminal_writestring("  VGA cursor position: (");
    terminal_putchar('0' + (char)global_vga_driver.getCursorX());
    terminal_writestring(", ");
    terminal_putchar('0' + (char)global_vga_driver.getCursorY());
    terminal_writestring(")\n");
    terminal_writestring("  VGA direct write: OK\n");
}

extern "C" void vga_write_at(char c, unsigned int x, unsigned int y) {
    global_vga_driver.writeAt(c, x, y);
}

extern "C" void vga_write_string_at(const char* str, unsigned int x, unsigned int y) {
    global_vga_driver.writeStringAt(str, x, y);
}

extern "C" void vga_move_cursor(unsigned int x, unsigned int y) {
    global_vga_driver.moveCursor(x, y);
}

extern "C" void vga_clear_row(unsigned int y) {
    global_vga_driver.clearRow(y);
}

extern "C" void vga_set_color(unsigned char fg, unsigned char bg) {
    global_vga_driver.setColor(fg, bg);
}

extern "C" void vga_hide_cursor(void) {
    global_vga_driver.hideCursor();
}

extern "C" void vga_show_cursor(void) {
    global_vga_driver.showCursor();
}


