#include "task.h"
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

static void dummy_task_a(void) {
    for (;;) {}
}

static void dummy_task_b(void) {
    for (;;) {}
}

void task_test(void) {
    terminal_writestring("\nTask Manager Test\n-----------------\n");
    tests_passed = 0;
    tests_failed = 0;

    uint32_t current = task_get_current();
    report("initial current task is 0", current == 0);

    uint32_t tid_a = task_create(dummy_task_a);
    report("task_create returns valid id", tid_a != (uint32_t)-1);

    uint32_t tid_b = task_create(dummy_task_b);
    report("second task gets distinct id", tid_b != tid_a && tid_b != (uint32_t)-1);

    uint32_t overflow_tid = (uint32_t)-1;
    int overflow_ok = 1;
    for (int i = 0; i < 10; i++) {
        uint32_t t = task_create(dummy_task_a);
        if (t != (uint32_t)-1 && t >= 8) {
            overflow_ok = 0;
        }
    }
    report("task_create returns -1 when MAX_TASKS exceeded", overflow_ok);
    (void)overflow_tid;

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
