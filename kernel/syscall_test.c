#include "syscall_wrapper.h"
#include "terminal.h"

void syscall_test(void) {
    terminal_writestring("\n=== System Call Interface Test ===\n");

    terminal_writestring("\n[TEST 1] getpid() syscall\n");
    int32_t pid = getpid();
    terminal_writestring("Process ID: ");
    char pid_buf[16];
    itoa_simple(pid, pid_buf);
    terminal_writestring(pid_buf);
    terminal_writestring("\n");

    terminal_writestring("\n[TEST 2] write() syscall to stdout\n");
    const char* msg = "Hello from syscall!\n";
    int32_t written = write(1, msg, 20);
    terminal_writestring("Bytes written: ");
    char write_buf[16];
    itoa_simple(written, write_buf);
    terminal_writestring(write_buf);
    terminal_writestring("\n");

    terminal_writestring("\n[TEST 3] Multiple write() calls\n");
    write(1, "Line 1\n", 7);
    write(1, "Line 2\n", 7);
    write(1, "Line 3\n", 7);

    terminal_writestring("\n[TEST 4] close() and open() syscalls\n");
    int32_t fd = open("/dev/null", 0);
    terminal_writestring("open() returned: ");
    char fd_buf[16];
    itoa_simple(fd, fd_buf);
    terminal_writestring(fd_buf);
    terminal_writestring("\n");

    int32_t close_ret = close(fd);
    terminal_writestring("close() returned: ");
    char close_buf[16];
    itoa_simple(close_ret, close_buf);
    terminal_writestring(close_buf);
    terminal_writestring("\n");

    terminal_writestring("\n[TEST 5] gettime() syscall\n");
    uint32_t time = 0;
    int32_t time_ret = gettime(&time);
    terminal_writestring("gettime() returned: ");
    char time_buf[16];
    itoa_simple(time_ret, time_buf);
    terminal_writestring(time_buf);
    terminal_writestring("\n");

    terminal_writestring("\n[TEST 6] Invalid syscall handling\n");
    int32_t invalid_ret = syscall0(99);
    terminal_writestring("Invalid syscall returned: ");
    char inv_buf[16];
    itoa_simple(invalid_ret, inv_buf);
    terminal_writestring(inv_buf);
    terminal_writestring("\n");

    terminal_writestring("\n=== All syscall tests completed ===\n");
}

void itoa_simple(int32_t val, char* buf);
