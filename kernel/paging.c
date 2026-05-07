#include "paging.h"
#include <stddef.h>

#define IDENTITY_MAP_TABLES     4U
#define IDENTITY_MAP_PAGES      (IDENTITY_MAP_TABLES * PAGE_TABLE_ENTRIES)

static page_directory_t kernel_directory __attribute__((aligned(PAGE_SIZE)));
static page_table_t     kernel_tables[PAGE_DIR_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static page_directory_t *current_directory = NULL;

extern void paging_enable(uint32_t page_directory_phys);

static uint32_t vaddr_to_pd_index(uint32_t vaddr) {
    return vaddr >> PAGE_DIR_SHIFT;
}

static uint32_t vaddr_to_pt_index(uint32_t vaddr) {
    return (vaddr >> PAGE_TABLE_SHIFT) & PAGE_TABLE_INDEX_MASK;
}

void paging_init(void) {
    for (uint32_t i = 0; i < PAGE_DIR_ENTRIES; i++) {
        kernel_directory.tables_physical[i] = 0;
        kernel_directory.tables[i] = NULL;
    }

    for (uint32_t i = 0; i < PAGE_DIR_ENTRIES; i++) {
        for (uint32_t j = 0; j < PAGE_TABLE_ENTRIES; j++) {
            kernel_tables[i].entries[j].present = 0;
            kernel_tables[i].entries[j].rw      = 1;
            kernel_tables[i].entries[j].user    = 0;
            kernel_tables[i].entries[j].frame   = 0;
        }
        kernel_directory.tables[i]          = &kernel_tables[i];
        kernel_directory.tables_physical[i] = ((uint32_t)&kernel_tables[i]) | PAGE_PRESENT | PAGE_WRITE;
    }

    for (uint32_t i = 0; i < IDENTITY_MAP_TABLES; i++) {
        for (uint32_t j = 0; j < PAGE_TABLE_ENTRIES; j++) {
            kernel_tables[i].entries[j].present = 1;
            kernel_tables[i].entries[j].rw      = 1;
            kernel_tables[i].entries[j].frame   = i * PAGE_TABLE_ENTRIES + j;
        }
    }

    current_directory = &kernel_directory;
    paging_enable((uint32_t)kernel_directory.tables_physical);
}

void paging_map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags) {
    uint32_t pd_idx = vaddr_to_pd_index(vaddr);
    uint32_t pt_idx = vaddr_to_pt_index(vaddr);

    if (current_directory->tables[pd_idx] == NULL)
        return;

    page_entry_t *entry = &current_directory->tables[pd_idx]->entries[pt_idx];
    entry->present = (flags & PAGE_PRESENT) ? 1 : 0;
    entry->rw      = (flags & PAGE_WRITE)   ? 1 : 0;
    entry->user    = (flags & PAGE_USER)    ? 1 : 0;
    entry->frame   = paddr >> PAGE_TABLE_SHIFT;
}

void paging_unmap_page(uint32_t vaddr) {
    uint32_t pd_idx = vaddr_to_pd_index(vaddr);
    uint32_t pt_idx = vaddr_to_pt_index(vaddr);

    if (current_directory->tables[pd_idx] == NULL)
        return;

    page_entry_t *entry = &current_directory->tables[pd_idx]->entries[pt_idx];
    entry->present = 0;
    entry->frame   = 0;
}

uint32_t paging_get_physical_address(uint32_t vaddr) {
    uint32_t pd_idx = vaddr_to_pd_index(vaddr);
    uint32_t pt_idx = vaddr_to_pt_index(vaddr);

    if (current_directory->tables[pd_idx] == NULL)
        return 0;

    page_entry_t *entry = &current_directory->tables[pd_idx]->entries[pt_idx];
    if (!entry->present)
        return 0;

    return (entry->frame << PAGE_TABLE_SHIFT) | (vaddr & PAGE_OFFSET_MASK);
}

page_directory_t *paging_get_current_directory(void) {
    return current_directory;
}
