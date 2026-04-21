#include "paging.h"
#include "terminal.h"

static int tests_passed = 0;
static int tests_failed = 0;

static void report(const char* name, int passed) {
    if (passed) {
        terminal_writestring("[PASS] ");
        tests_passed++;
    } else {
        terminal_writestring("[FAIL] ");
        tests_failed++;
    }
    terminal_writestring(name);
    terminal_writestring("\n");
}

void paging_test(void) {
    terminal_writestring("\nPaging Test\n-----------\n");
    tests_passed = 0;
    tests_failed = 0;

    page_directory_t* dir = paging_get_current_directory();
    report("current directory is non-null", dir != (void*)0);

    uint32_t vaddr = 0x00400000;
    uint32_t paddr = 0x00200000;

    paging_map_page(vaddr, paddr, PAGE_PRESENT | PAGE_WRITE);
    uint32_t resolved = paging_get_physical_address(vaddr);
    report("mapped page resolves to correct physical address", resolved == paddr);

    paging_unmap_page(vaddr);
    uint32_t after_unmap = paging_get_physical_address(vaddr);
    report("unmapped page resolves to 0", after_unmap == 0);

    uint32_t unmapped_vaddr = 0x80000000;
    uint32_t result = paging_get_physical_address(unmapped_vaddr);
    report("unmapped high address returns 0", result == 0);

    uint32_t vaddr2 = 0x00401000;
    paging_map_page(vaddr2, paddr + 0x1000, PAGE_PRESENT);
    uint32_t resolved2 = paging_get_physical_address(vaddr2);
    report("second distinct page maps independently", resolved2 == paddr + 0x1000);
    paging_unmap_page(vaddr2);

    terminal_writestring("Results: ");
    char buf[8];
    int v = tests_passed;
    buf[0] = '0' + v / 10;
    buf[1] = '0' + v % 10;
    buf[2] = '/';
    int t = tests_passed + tests_failed;
    buf[3] = '0' + t / 10;
    buf[4] = '0' + t % 10;
    buf[5] = '\n';
    buf[6] = '\0';
    terminal_writestring(buf);
}
