#ifndef PAGING_H
#define PAGING_H

#include "memory.h"

#define PAGE_PRESENT    0x1U
#define PAGE_WRITE      0x2U
#define PAGE_USER       0x4U

void              paging_init(void);
void              paging_map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags);
void              paging_unmap_page(uint32_t vaddr);
uint32_t          paging_get_physical_address(uint32_t vaddr);
page_directory_t *paging_get_current_directory(void);

#endif

