#include <stdint.h>
#include <stdbool.h>
#include "terminal.h"
#include "string.h"

#define SHELL_BUFFER_SIZE 256

static char command_buffer[SHELL_BUFFER_SIZE];
static uint32_t buffer_pos = 0;

extern void terminal_writestring(const char* str);
extern void terminal_putchar(char c);
extern void terminal_setcolor(uint8_t color);
extern int strcmp(const char* s1, const char* s2);

static void shell_prompt(void) {
    terminal_setcolor(0x0A);
    terminal_writestring("toyos> ");
    terminal_setcolor(0x0F);
}

static void clear_screen_cmd(void) {
    extern void terminal_initialize(void);
    terminal_initialize();
}

static void help_cmd(void) {
    terminal_writestring("Available commands:\n");
    terminal_writestring("  help     - Show this message\n");
    terminal_writestring("  clear    - Clear screen\n");
    terminal_writestring("  version  - Show OS version\n");
    terminal_writestring("  meminfo  - Display memory stats\n");
    terminal_writestring("  time     - Show system uptime\n");
    terminal_writestring("  echo     - Echo arguments\n");
    terminal_writestring("  shutdown - Power off\n");
    terminal_writestring("  reboot   - Restart system\n");
}

static void version_cmd(void) {
    terminal_writestring("ToyOS v0.1\n");
    terminal_writestring("Multi-language kernel\n");
}

static void meminfo_cmd(void) {
    extern void rust_print_stats(void);
    terminal_writestring("Memory Information:\n");
    rust_print_stats();
}

static void time_cmd(void) {
    extern uint32_t timer_ticks;
    terminal_writestring("System uptime: ");
    uint32_t seconds = timer_ticks / 100;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    char buf[16];
    buf[0] = '0' + (hours / 10);
    buf[1] = '0' + (hours % 10);
    buf[2] = ':';
    buf[3] = '0' + ((minutes % 60) / 10);
    buf[4] = '0' + ((minutes % 60) % 10);
    buf[5] = ':';
    buf[6] = '0' + ((seconds % 60) / 10);
    buf[7] = '0' + ((seconds % 60) % 10);
    buf[8] = '\0';
    terminal_writestring(buf);
    terminal_writestring("\n");
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

void shell_handle_input(char c) {
    if (c == '\n') {
        terminal_putchar('\n');
        parse_and_execute();
        buffer_pos = 0;
        shell_prompt();
    } else if (c == '\b') {
        if (buffer_pos > 0) {
            buffer_pos--;
            terminal_putchar('\b');
        }
    } else if (buffer_pos < SHELL_BUFFER_SIZE - 1) {
        command_buffer[buffer_pos++] = c;
        terminal_putchar(c);
    }
}
