#include <stdint.h>
#include "terminal.h"
#include "port.h"

typedef struct {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

extern void terminal_writestring(const char* str);
extern void terminal_setcolor(uint8_t color);

/* Print a 32-bit value as zero-padded hex (no stdlib available). */
static void print_hex(uint32_t val) {
    static const char digits[] = "0123456789ABCDEF";
    char buf[8];
    for (int i = 7; i >= 0; i--) {
        buf[i] = digits[val & 0xF];
        val >>= 4;
    }
    for (int i = 0; i < 8; i++) {
        char tmp[2] = { buf[i], '\0' };
        terminal_writestring(tmp);
    }
}

static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check"
};

void isr_handler(registers_t* regs) {
    if (regs->int_no < 32) {
        terminal_setcolor(0x4F);
        terminal_writestring("\n=== KERNEL PANIC ===\n");
        terminal_writestring("Exception #");
        /* Print int_no as decimal (0-31, always two digits max) */
        char num[3] = { '0' + (char)(regs->int_no / 10),
                        '0' + (char)(regs->int_no % 10), '\0' };
        if (regs->int_no < 10) {
            terminal_writestring(num + 1);
        } else {
            terminal_writestring(num);
        }
        terminal_writestring(" - ");
        if (regs->int_no < 19) {
            terminal_writestring(exception_messages[regs->int_no]);
        } else {
            terminal_writestring("Reserved");
        }
        terminal_writestring("\nEIP: 0x"); print_hex(regs->eip);
        terminal_writestring("  ERR: 0x"); print_hex(regs->err_code);
        terminal_writestring("\nCS:  0x"); print_hex(regs->cs);
        terminal_writestring("  EFLAGS: 0x"); print_hex(regs->eflags);
        terminal_writestring("\n");
        for (;;);
    }
}

void irq_handler(registers_t* regs) {
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
    
    if (regs->int_no == 32) {
        extern void timer_handler(void);
        timer_handler();
    } else if (regs->int_no == 33) {
        extern void keyboard_handler(void);
        keyboard_handler();
    }
}
