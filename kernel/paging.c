#include "paging.h"
#include <stddef.h>

static page_directory_t kernel_directory __attribute__((aligned(4096)));
static page_table_t kernel_tables[1024] __attribute__((aligned(4096)));
static page_directory_t* current_directory = NULL;

extern void paging_enable(uint32_t page_directory);

static uint32_t get_page_directory_index(uint32_t virtual_addr) {
    return virtual_addr >> 22;
}

static uint32_t get_page_table_index(uint32_t virtual_addr) {
    return (virtual_addr >> 12) & 0x3FF;
}

void paging_init(void) {
    for (int i = 0; i < 1024; i++) {
        kernel_directory.tables_physical[i] = 0;
        kernel_directory.tables[i] = NULL;
    }
    
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 1024; j++) {
            kernel_tables[i].pages[j].present = 0;
            kernel_tables[i].pages[j].rw = 1;
            kernel_tables[i].pages[j].user = 0;
            kernel_tables[i].pages[j].frame = 0;
        }
        kernel_directory.tables[i] = &kernel_tables[i];
        kernel_directory.tables_physical[i] = ((uint32_t)&kernel_tables[i]) | PAGE_PRESENT | PAGE_WRITE;
    }
    
    current_directory = &kernel_directory;
    paging_enable((uint32_t)kernel_directory.tables_physical);
}

void paging_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    uint32_t pd_index = get_page_directory_index(virtual_addr);
    uint32_t pt_index = get_page_table_index(virtual_addr);
    
    if (current_directory->tables[pd_index] == NULL) {
        return;
    }
    
    page_t* page = &current_directory->tables[pd_index]->pages[pt_index];
    page->present = (flags & PAGE_PRESENT) ? 1 : 0;
    page->rw = (flags & PAGE_WRITE) ? 1 : 0;
    page->user = (flags & PAGE_USER) ? 1 : 0;
    page->frame = physical_addr >> 12;
}

void paging_unmap_page(uint32_t virtual_addr) {
    uint32_t pd_index = get_page_directory_index(virtual_addr);
    uint32_t pt_index = get_page_table_index(virtual_addr);
    
    if (current_directory->tables[pd_index] == NULL) {
        return;
    }
    
    page_t* page = &current_directory->tables[pd_index]->pages[pt_index];
    page->present = 0;
    page->frame = 0;
}

uint32_t paging_get_physical_address(uint32_t virtual_addr) {
    uint32_t pd_index = get_page_directory_index(virtual_addr);
    uint32_t pt_index = get_page_table_index(virtual_addr);
    
    if (current_directory->tables[pd_index] == NULL) {
        return 0;
    }
    
    page_t* page = &current_directory->tables[pd_index]->pages[pt_index];
    if (!page->present) {
        return 0;
    }
    
    return (page->frame << 12) | (virtual_addr & 0xFFF);
}

page_directory_t* paging_get_current_directory(void) {
    return current_directory;
}
