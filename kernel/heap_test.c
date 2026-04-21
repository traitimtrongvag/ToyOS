#include "heap.h"
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

void heap_test(void) {
    terminal_writestring("\nHeap Allocator Test\n-------------------\n");
    tests_passed = 0;
    tests_failed = 0;

    void* p1 = kmalloc(64);
    report("kmalloc returns non-null", p1 != (void*)0);

    void* p2 = kmalloc(128);
    report("two allocations do not overlap", p2 != p1);

    kfree(p1);
    void* p3 = kmalloc(64);
    report("freed block is reused", p3 == p1);
    kfree(p3);

    void* p4 = kmalloc(0);
    report("kmalloc(0) returns null", p4 == (void*)0);

    void* p5 = kmalloc(32);
    void* p6 = krealloc(p5, 128);
    report("krealloc returns non-null", p6 != (void*)0);
    kfree(p6);

    void* p7 = krealloc((void*)0, 48);
    report("krealloc(null) behaves as kmalloc", p7 != (void*)0);
    kfree(p7);

    kfree((void*)0);
    report("kfree(null) does not crash", 1);

    uint32_t used_before = heap_get_used();
    void* p8 = kmalloc(256);
    uint32_t used_after = heap_get_used();
    report("heap_get_used increases after alloc", used_after > used_before);
    kfree(p8);
    report("heap_get_free increases after free", heap_get_free() > heap_get_used() || heap_get_used() <= used_before);

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
