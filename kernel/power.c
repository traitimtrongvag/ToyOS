#include <stdint.h>
#include "port.h"

#define ACPI_POWER_OFF 0x2000
#define QEMU_SHUTDOWN_PORT 0x604

void acpi_power_off(void) {
    outw(QEMU_SHUTDOWN_PORT, ACPI_POWER_OFF);
    for (;;) __asm__ volatile("hlt");
}

void reboot(void) {
    uint8_t temp;
    __asm__ volatile("cli");
    /* Drain keyboard controller input buffer */
    do {
        temp = inb(0x64);
        if (temp & 0x01) inb(0x60);
    } while (temp & 0x02);
    /* Pulse CPU reset line via keyboard controller */
    outb(0x64, 0xFE);
    /* Short spin to let the reset propagate */
    for (volatile int i = 0; i < 100000; i++) {}
    /* Fallback: triple-fault via null IDT load — guaranteed reset on QEMU */
    struct { uint16_t limit; uint32_t base; } __attribute__((packed)) null_idt = {0, 0};
    __asm__ volatile("lidt %0" : : "m"(null_idt));
    __asm__ volatile("int $0x3");
    for (;;) __asm__ volatile("hlt");
}

void halt(void) {
    __asm__ volatile("cli");
    for (;;) __asm__ volatile("hlt");
}
