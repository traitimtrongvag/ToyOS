#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_PAGE_NUMBER (KERNEL_VIRTUAL_BASE >> 22)

#define PAGE_SIZE 4096
#define PAGE_ALIGN_DOWN(x) ((x) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(x) (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

typedef struct {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;
} page_t;

typedef struct {
    page_t pages[1024];
} page_table_t;

typedef struct {
    uint32_t tables_physical[1024];
    page_table_t* tables[1024];
    uint32_t physical_addr;
} page_directory_t;

#endif
