#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define SYSCALL_INT 0x80

#define SYS_EXIT      0
#define SYS_WRITE     1
#define SYS_READ      2
#define SYS_OPEN      3
#define SYS_CLOSE     4
#define SYS_GETPID    5
#define SYS_SLEEP     6
#define SYS_GETTIME   7
#define SYS_SBRK      8
#define SYS_MMAP      9

#define MAX_SYSCALLS  10

typedef int32_t (*syscall_handler_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

void syscall_init(void);
int32_t syscall_dispatch(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);

#endif
