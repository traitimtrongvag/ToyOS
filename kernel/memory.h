#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define KERNEL_VIRTUAL_BASE     0xC0000000U
#define KERNEL_PAGE_NUMBER      (KERNEL_VIRTUAL_BASE >> 22)

#define PAGE_SIZE               4096U
#define PAGE_TABLE_ENTRIES      1024U
#define PAGE_DIR_ENTRIES        1024U

#define PAGE_ADDR_MASK          0xFFFFF000U
#define PAGE_OFFSET_MASK        0x00000FFFU
#define PAGE_DIR_SHIFT          22U
#define PAGE_TABLE_SHIFT        12U
#define PAGE_TABLE_INDEX_MASK   0x3FFU

#define PAGE_ALIGN_DOWN(x)      ((x) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(x)        (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

typedef struct {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;
} page_entry_t;

typedef struct {
    page_entry_t entries[PAGE_TABLE_ENTRIES];
} page_table_t;

typedef struct {
    uint32_t     tables_physical[PAGE_DIR_ENTRIES];
    page_table_t *tables[PAGE_DIR_ENTRIES];
    uint32_t     physical_addr;
} page_directory_t;

#endif
