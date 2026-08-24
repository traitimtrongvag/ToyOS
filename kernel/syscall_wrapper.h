#ifndef SYSCALL_WRAPPER_H
#define SYSCALL_WRAPPER_H

#include <stdint.h>
#include "syscall.h"

static inline int32_t syscall0(uint32_t num) {
    int32_t ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(num)
        : "memory"
    );
    return ret;
}

static inline int32_t syscall1(uint32_t num, uint32_t arg1) {
    int32_t ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1)
        : "memory"
    );
    return ret;
}

static inline int32_t syscall2(uint32_t num, uint32_t arg1, uint32_t arg2) {
    int32_t ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2)
        : "memory"
    );
    return ret;
}

static inline int32_t syscall3(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    int32_t ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
        : "memory"
    );
    return ret;
}

#define exit(code)    syscall1(SYS_EXIT, code)
#define write(fd, buf, count) syscall3(SYS_WRITE, fd, (uint32_t)(buf), count)
#define read(fd, buf, count)  syscall3(SYS_READ, fd, (uint32_t)(buf), count)
#define open(path, flags)     syscall2(SYS_OPEN, (uint32_t)(path), flags)
#define close(fd)             syscall1(SYS_CLOSE, fd)
#define getpid()              syscall0(SYS_GETPID)
#define sleep(ms)             syscall1(SYS_SLEEP, ms)
#define gettime(ptr)          syscall1(SYS_GETTIME, (uint32_t)(ptr))
#define sbrk(inc)             syscall1(SYS_SBRK, inc)
#define mmap(addr, len)       syscall2(SYS_MMAP, addr, len)

#endif
