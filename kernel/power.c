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
    do {
        temp = inb(0x64);
        if (temp & 0x01) inb(0x60);
    } while (temp & 0x02);
    outb(0x64, 0xFE);
    for (;;) __asm__ volatile("hlt");
}

void halt(void) {
    __asm__ volatile("cli");
    for (;;) __asm__ volatile("hlt");
}
