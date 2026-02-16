#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_ENTRIES 5

#define GDT_ACCESS_PRESENT 0x80
#define GDT_ACCESS_RING0 0x00
#define GDT_ACCESS_RING3 0x60
#define GDT_ACCESS_CODE_EXEC 0x18
#define GDT_ACCESS_DATA_RW 0x12

#define GDT_GRANULARITY_4K 0xC0
#define GDT_GRANULARITY_BYTE 0x40
#define GDT_GRANULARITY_32BIT 0x0F

void gdt_init(void);
void gdt_install(void);

#endif
