#include "syscall.h"
#include "idt.h"
#include "terminal.h"
#include "string.h"

extern void syscall_handler(void);

static syscall_handler_t syscall_table[MAX_SYSCALLS];
static uint32_t next_pid = 1;

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

static int32_t sys_gettime(uint32_t time_ptr) {
    if (time_ptr) {
        uint32_t* ptr = (uint32_t*)time_ptr;
        *ptr = 0;
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

    idt_set_gate(SYSCALL_INT, (uint32_t)syscall_handler, 0x08, 0xEE);
}

void itoa_simple(int32_t val, char* buf) {
    int i = 0;
    int is_negative = 0;

    if (val == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    if (val < 0) {
        is_negative = 1;
        val = -val;
    }

    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    buf[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char temp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = temp;
    }
}
