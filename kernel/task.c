#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "task.h"

#define MAX_TASKS           8U
#define TASK_STACK_WORDS    1024U
#define EFLAGS_IF           0x00000202U
#define KERNEL_CODE_SEG     0x08U
#define STACK_PADDING_REGS  8U

typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp;
    uint32_t eip, esp;
    uint32_t eflags;
} task_context_t;

typedef struct {
    uint32_t       id;
    task_context_t context;
    uint32_t      *stack_base;
    task_state_t   state;
} task_t;

static task_t  tasks[MAX_TASKS];
static uint32_t current_task = 0;
static volatile uint32_t task_count = 0;
static uint32_t task_stacks[MAX_TASKS][TASK_STACK_WORDS] __attribute__((aligned(16)));

uint32_t task_create(void (*entry)(void)) {
    if (task_count >= MAX_TASKS)
        return (uint32_t)-1;

    uint32_t tid   = task_count++;
    task_t  *task  = &tasks[tid];

    task->id         = tid;
    task->state      = TASK_READY;
    task->stack_base = task_stacks[tid];

    uint32_t *stack_top = &task_stacks[tid][TASK_STACK_WORDS];
    *--stack_top = EFLAGS_IF;
    *--stack_top = KERNEL_CODE_SEG;
    *--stack_top = (uint32_t)entry;
    for (int i = 0; i < STACK_PADDING_REGS; i++)
        *--stack_top = 0;

    task->context.esp    = (uint32_t)stack_top;
    task->context.eip    = (uint32_t)entry;
    task->context.eflags = EFLAGS_IF;

    return tid;
}

void task_switch(void) {
    uint32_t prev_task = current_task;
    do {
        current_task = (current_task + 1) % task_count;
    } while (tasks[current_task].state != TASK_READY && current_task != prev_task);

    if (current_task == prev_task)
        return;

    __asm__ volatile(
        "pushf\n\t" "cli\n\t" "pusha\n\t"
        "mov %%esp, %0\n\t"
        "mov %1, %%esp\n\t"
        "popa\n\t" "popf\n\t"
        : "=m"(tasks[prev_task].context.esp)
        : "m"(tasks[current_task].context.esp)
    );
}

void task_yield(void)          { task_switch(); }
uint32_t task_get_current(void) { return current_task; }

void task_init(void) {
    task_count   = 1;
    current_task = 0;

    tasks[0].id         = 0;
    tasks[0].state      = TASK_RUNNING;
    tasks[0].stack_base = NULL;
}
