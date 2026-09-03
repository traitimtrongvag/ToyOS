#include <stdint.h>
#include <stdbool.h>
#include "terminal.h"
#include "string.h"
#include "power.h"

#define SHELL_BUFFER_SIZE 256

static char command_buffer[SHELL_BUFFER_SIZE];
static uint32_t buffer_pos = 0;  /* total chars in buffer */
static uint32_t cursor_pos = 0;  /* cursor position within buffer (0..buffer_pos) */

static void shell_prompt(void) {
    terminal_setcolor(0x0A);
    terminal_writestring("toyos> ");
    terminal_setcolor(0x0F);
}

static void clear_screen_cmd(void) {
    terminal_initialize();
}

static void help_cmd(void) {
    terminal_writestring("Available commands:\n");
    terminal_writestring("  help         - Show this message\n");
    terminal_writestring("  clear        - Clear screen\n");
    terminal_writestring("  version      - Show OS version\n");
    terminal_writestring("  meminfo      - Display memory stats\n");
    terminal_writestring("  time         - Show system uptime\n");
    terminal_writestring("  rtc          - Read hardware RTC clock\n");
    terminal_writestring("  vfs          - Run VFS demo (create/write/read)\n");
    terminal_writestring("  syscall-test - Run syscall interface tests\n");
    terminal_writestring("  echo         - Echo arguments\n");
    terminal_writestring("  shutdown     - Power off\n");
    terminal_writestring("  reboot       - Restart system\n");
}

static void version_cmd(void) {
    terminal_writestring("ToyOS v0.1\n");
}

static void meminfo_cmd(void) {
    extern void rust_print_stats(void);
    extern uint32_t heap_get_used(void);
    extern uint32_t heap_get_free(void);

    terminal_writestring("=== Memory Info ===\n");

    /* Kernel heap (kmalloc pool) */
    terminal_writestring("Heap used : ");
    uint32_t used = heap_get_used();
    /* Print used bytes via simple digit loop to avoid compiler-rt division */
    static const uint32_t POW10[] = {
        1000000000u, 100000000u, 10000000u, 1000000u,
        100000u, 10000u, 1000u, 100u, 10u, 1u
    };
    int started = 0;
    uint32_t v = used;
    for (int i = 0; i < 10; i++) {
        if (!started && v < POW10[i]) continue;
        started = 1;
        int d = 0;
        while (v >= POW10[i]) { v -= POW10[i]; d++; }
        terminal_putchar('0' + d);
    }
    if (!started) terminal_putchar('0');
    terminal_writestring(" bytes\n");

    terminal_writestring("Heap free : ");
    started = 0;
    v = heap_get_free();
    for (int i = 0; i < 10; i++) {
        if (!started && v < POW10[i]) continue;
        started = 1;
        int d = 0;
        while (v >= POW10[i]) { v -= POW10[i]; d++; }
        terminal_putchar('0' + d);
    }
    if (!started) terminal_putchar('0');
    terminal_writestring(" bytes\n");

    /* Rust page allocator stats */
    terminal_writestring("Page allocator:\n");
    rust_print_stats();
}

/* div_u32: integer divide n/d without / operator (avoids __divsi3).
 * Returns quotient; stores remainder in *rem. */
static uint32_t div_u32(uint32_t n, uint32_t d, uint32_t *rem) {
    uint32_t q = 0;
    while (n >= d) { n -= d; q++; }
    if (rem) *rem = n;
    return q;
}

static void time_cmd(void) {
    extern uint32_t timer_ticks;
    terminal_writestring("System uptime: ");
    uint32_t seconds = div_u32(timer_ticks, 100, 0);
    uint32_t sec_rem;
    uint32_t total_min = div_u32(seconds, 60, &sec_rem);
    uint32_t min_rem;
    uint32_t hours = div_u32(total_min, 60, &min_rem);
    uint32_t h10, h1_r, m10, m1_r, s10, s1_r;
    h10  = div_u32(hours,   10, &h1_r);
    m10  = div_u32(min_rem, 10, &m1_r);
    s10  = div_u32(sec_rem, 10, &s1_r);
    char buf[9];
    buf[0] = '0' + (uint8_t)h10;
    buf[1] = '0' + (uint8_t)h1_r;
    buf[2] = ':';
    buf[3] = '0' + (uint8_t)m10;
    buf[4] = '0' + (uint8_t)m1_r;
    buf[5] = ':';
    buf[6] = '0' + (uint8_t)s10;
    buf[7] = '0' + (uint8_t)s1_r;
    buf[8] = '\0';
    terminal_writestring(buf);
    terminal_writestring("\n");
}

static void print_two_digits(uint8_t val) {
    char buf[3];
    uint32_t hi, lo;
    hi = div_u32(val, 10, &lo);
    buf[0] = '0' + (uint8_t)hi;
    buf[1] = '0' + (uint8_t)lo;
    buf[2] = '\0';
    terminal_writestring(buf);
}

static void print_year(uint16_t year) {
    char buf[5];
    uint32_t y = year, r1, r2, r3, d0, d1, d2, d3;
    d0 = div_u32(y,  1000, &r1);
    d1 = div_u32(r1, 100,  &r2);
    d2 = div_u32(r2, 10,   &r3);
    d3 = r3;
    buf[0] = '0' + (uint8_t)d0;
    buf[1] = '0' + (uint8_t)d1;
    buf[2] = '0' + (uint8_t)d2;
    buf[3] = '0' + (uint8_t)d3;
    buf[4] = '\0';
    terminal_writestring(buf);
}

static void rtc_cmd(void) {
    typedef struct {
        uint8_t second;
        uint8_t minute;
        uint8_t hour;
        uint8_t day;
        uint8_t month;
        uint16_t year;
    } rtc_time_t;

    extern void rtc_read(rtc_time_t* time);
    rtc_time_t t;
    rtc_read(&t);

    terminal_writestring("RTC: ");
    print_year(t.year);
    terminal_putchar('-');
    print_two_digits(t.month);
    terminal_putchar('-');
    print_two_digits(t.day);
    terminal_putchar(' ');
    print_two_digits(t.hour);
    terminal_putchar(':');
    print_two_digits(t.minute);
    terminal_putchar(':');
    print_two_digits(t.second);
    terminal_putchar('\n');
}

static void vfs_cmd(void) {
    extern void vfs_demo(void);
    vfs_demo();
}

static void syscall_test_cmd(void) {
    extern void syscall_test(void);
    syscall_test();
}

static void echo_cmd(const char* args) {
    if (*args == '\0') {
        terminal_writestring("\n");
        return;
    }
    terminal_writestring(args);
    terminal_writestring("\n");
}


static void parse_and_execute(void) {
    if (buffer_pos == 0) return;
    if (buffer_pos >= SHELL_BUFFER_SIZE) buffer_pos = SHELL_BUFFER_SIZE - 1;
    command_buffer[buffer_pos] = '\0';
    char* cmd = command_buffer;
    while (*cmd == ' ') cmd++;
    char* args = cmd;
    while (*args && *args != ' ') args++;
    if (*args) {
        *args = '\0';
        args++;
        while (*args == ' ') args++;
    }

    if (strcmp(cmd, "help") == 0) {
        help_cmd();
    } else if (strcmp(cmd, "clear") == 0) {
        clear_screen_cmd();
        return;
    } else if (strcmp(cmd, "version") == 0) {
        version_cmd();
    } else if (strcmp(cmd, "meminfo") == 0) {
        meminfo_cmd();
    } else if (strcmp(cmd, "time") == 0) {
        time_cmd();
    } else if (strcmp(cmd, "rtc") == 0) {
        rtc_cmd();
    } else if (strcmp(cmd, "vfs") == 0) {
        vfs_cmd();
    } else if (strcmp(cmd, "syscall-test") == 0) {
        syscall_test_cmd();
    } else if (strcmp(cmd, "echo") == 0) {
        echo_cmd(args);
    } else if (strcmp(cmd, "shutdown") == 0) {
        terminal_setcolor(0x0C);
        terminal_writestring("Shutting down...\n");
        extern void acpi_power_off(void);
        acpi_power_off();
    } else if (strcmp(cmd, "reboot") == 0) {
        terminal_setcolor(0x0C);
        terminal_writestring("Rebooting...\n");
        extern void reboot(void);
        reboot();
    } else if (*cmd != '\0') {
        terminal_writestring("Unknown command: ");
        terminal_writestring(cmd);
        terminal_writestring("\nType 'help' for available commands.\n");
    }
}

void shell_init(void) {
    buffer_pos = 0;
    terminal_writestring("\nWelcome to ToyOS Shell!\n");
    terminal_writestring("Type 'help' for available commands.\n\n");
    shell_prompt();
}

#define HISTORY_SIZE 16

static char history[HISTORY_SIZE][SHELL_BUFFER_SIZE];
static int  history_count = 0;   /* total entries saved, max HISTORY_SIZE */
static int  history_idx   = -1;  /* -1 = not browsing history */

/* ANSI escape parser state */
#define ESC_NONE  0
#define ESC_GOT_ESC  1
#define ESC_GOT_BRACKET 2
static int esc_state = ESC_NONE;

static void history_push(const char* cmd) {
    if (cmd[0] == '\0') return;
    /* Don't push duplicate of last entry */
    if (history_count > 0) {
        int last = (history_count - 1) % HISTORY_SIZE;
        int same = 1;
        for (int i = 0; i < SHELL_BUFFER_SIZE; i++) {
            if (history[last][i] != cmd[i]) { same = 0; break; }
            if (cmd[i] == '\0') break;
        }
        if (same) return;
    }
    int slot = history_count % HISTORY_SIZE;
    for (int i = 0; i < SHELL_BUFFER_SIZE - 1 && cmd[i]; i++)
        history[slot][i] = cmd[i];
    history[slot][SHELL_BUFFER_SIZE - 1] = '\0';
    history_count++;
}

/* Erase current input line and replace with new string */
static void replace_line(const char* newcmd) {
    /* Erase chars already on screen */
    while (buffer_pos > 0) {
        terminal_putchar('\b');
        terminal_putchar(' ');
        terminal_putchar('\b');
        buffer_pos--;
    }
    /* Write new command */
    for (int i = 0; newcmd[i] && i < SHELL_BUFFER_SIZE - 1; i++) {
        command_buffer[buffer_pos++] = newcmd[i];
        terminal_putchar(newcmd[i]);
    }
    cursor_pos = buffer_pos;
}

void shell_handle_input(char c) {
    /* ANSI escape sequence: ESC [ A/B/C/D */
    if (esc_state == ESC_NONE && c == 0x1B) {
        esc_state = ESC_GOT_ESC;
        return;
    }
    if (esc_state == ESC_GOT_ESC) {
        if (c == '[') { esc_state = ESC_GOT_BRACKET; return; }
        esc_state = ESC_NONE;
        return;
    }
    if (esc_state == ESC_GOT_BRACKET) {
        esc_state = ESC_NONE;
        if (c == 'A') {
            /* Arrow UP - older history */
            if (history_count == 0) return;
            if (history_idx == -1)
                history_idx = history_count - 1;
            else if (history_idx > 0)
                history_idx--;
            int slot = history_idx % HISTORY_SIZE;
            replace_line(history[slot]);
        } else if (c == 'B') {
            /* Arrow DOWN - newer history */
            if (history_idx == -1) return;
            if (history_idx < history_count - 1) {
                history_idx++;
                int slot = history_idx % HISTORY_SIZE;
                replace_line(history[slot]);
            } else {
                /* Past newest - clear line */
                replace_line("");
                history_idx = -1;
            }
        } else if (c == 'C') {
            /* Arrow RIGHT - move cursor forward, but not past end of input */
            if (cursor_pos < buffer_pos) {
                cursor_pos++;
                terminal_move_cursor_right();
            }
        } else if (c == 'D') {
            /* Arrow LEFT - move cursor back, but not before start of input */
            if (cursor_pos > 0) {
                cursor_pos--;
                terminal_move_cursor_left();
            }
        }
        return;
    }

    if (c == '\n') {
        terminal_putchar('\n');
    if (buffer_pos >= SHELL_BUFFER_SIZE) buffer_pos = SHELL_BUFFER_SIZE - 1;
        command_buffer[buffer_pos] = '\0';
        history_push(command_buffer);
        history_idx = -1;
        parse_and_execute();
        buffer_pos = 0;
        cursor_pos = 0;
        shell_prompt();
    } else if (c == '\b') {
        if (buffer_pos > 0) {
            buffer_pos--;
            cursor_pos = buffer_pos;
            terminal_putchar('\b');
            terminal_putchar(' ');
            terminal_putchar('\b');
        }
    } else if (buffer_pos < SHELL_BUFFER_SIZE - 1) {
        command_buffer[buffer_pos++] = c;
        cursor_pos = buffer_pos;
        terminal_putchar(c);
    }
}
