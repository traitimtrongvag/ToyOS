#include "syscall.h"
#include "idt.h"
#include "terminal.h"
#include "string.h"

extern void syscall_handler(void);

static syscall_handler_t syscall_table[MAX_SYSCALLS];
static uint32_t next_pid = 1;

void itoa_simple(int32_t val, char* buf);

static int32_t sys_exit(uint32_t code) {
    terminal_writestring("[SYSCALL] Process exit with code: ");
    char buf[16];
    itoa_simple((int32_t)code, buf);
    terminal_writestring(buf);
    terminal_putchar('\n');
    return 0;
}

static int32_t sys_write(uint32_t fd, uint32_t buf, uint32_t count) {
    if (fd == 1 || fd == 2) {
        const char* str = (const char*)buf;
        for (uint32_t i = 0; i < count; i++) {
            terminal_putchar(str[i]);
        }
        return (int32_t)count;
    }
    return -1;
}

static int32_t sys_read(uint32_t fd, uint32_t buf, uint32_t count) {
    (void)fd;
    (void)buf;
    (void)count;
    return -1;
}

static int32_t sys_open(uint32_t path, uint32_t flags) {
    (void)path;
    (void)flags;
    return -1;
}

static int32_t sys_close(uint32_t fd) {
    (void)fd;
    return 0;
}

static int32_t sys_getpid(void) {
    return (int32_t)next_pid;
}

static int32_t sys_sleep(uint32_t milliseconds) {
    (void)milliseconds;
    return 0;
}

extern uint32_t timer_ticks;

static int32_t sys_gettime(uint32_t time_ptr) {
    if (time_ptr) {
        uint32_t* ptr = (uint32_t*)time_ptr;
        *ptr = timer_ticks;
    }
    return 0;
}

static int32_t sys_sbrk(uint32_t increment) {
    (void)increment;
    return -1;
}

static int32_t sys_mmap(uint32_t addr, uint32_t length) {
    (void)addr;
    (void)length;
    return -1;
}

int32_t syscall_dispatch(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) {
    (void)arg4;
    (void)arg5;

    if (num >= MAX_SYSCALLS || syscall_table[num] == NULL) {
        return -1;
    }

    return syscall_table[num](arg1, arg2, arg3, arg4, arg5);
}

void syscall_init(void) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
    syscall_table[SYS_EXIT] = (syscall_handler_t)sys_exit;
    syscall_table[SYS_WRITE] = (syscall_handler_t)sys_write;
    syscall_table[SYS_READ] = (syscall_handler_t)sys_read;
    syscall_table[SYS_OPEN] = (syscall_handler_t)sys_open;
    syscall_table[SYS_CLOSE] = (syscall_handler_t)sys_close;
    syscall_table[SYS_GETPID] = (syscall_handler_t)sys_getpid;
    syscall_table[SYS_SLEEP] = (syscall_handler_t)sys_sleep;
    syscall_table[SYS_GETTIME] = (syscall_handler_t)sys_gettime;
    syscall_table[SYS_SBRK] = (syscall_handler_t)sys_sbrk;
    syscall_table[SYS_MMAP] = (syscall_handler_t)sys_mmap;
#pragma GCC diagnostic pop

    idt_set_gate(SYSCALL_INT, (uint32_t)syscall_handler, 0x08, 0xEE);
}

/*
 * itoa_simple: convert int32_t to decimal string without using / or %.
 *
 * Division on i686 baremetal calls __divsi3 from compiler-rt which is not
 * linked in a freestanding build. Use a powers-of-ten subtraction loop
 * identical to the approach used in the Rust print_u32 fix.
 */
void itoa_simple(int32_t val, char* buf) {
    static const uint32_t powers[] = {
        1000000000u, 100000000u, 10000000u, 1000000u,
        100000u,     10000u,     1000u,     100u,
        10u,         1u
    };
    static const int NUM_POWERS = 10;

    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }

    int i = 0;
    uint32_t uval;
    if (val < 0) { buf[i++] = '-'; uval = (uint32_t)(-(val + 1)) + 1u; }
    else          { uval = (uint32_t)val; }

    int started = 0;
    for (int p = 0; p < NUM_POWERS; p++) {
        uint32_t power = powers[p];
        if (!started && uval < power) continue;
        started = 1;
        int digit = 0;
        while (uval >= power) { uval -= power; digit++; }
        buf[i++] = '0' + digit;
    }
    buf[i] = '\0';
}
