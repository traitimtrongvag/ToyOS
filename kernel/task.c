#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "task.h"

#define MAX_TASKS        8
#define TASK_STACK_WORDS 1024   /* 4KB per task stack */

typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp;
    uint32_t eip, esp;
    uint32_t eflags;
} task_context_t;

typedef struct {
    uint32_t id;
    task_context_t context;
    uint32_t* stack;
    task_state_t state;
} task_t;

static task_t tasks[MAX_TASKS];
static uint32_t current_task = 0;
/* volatile: read in task_switch (timer ISR context), written in task_create.
 * Safe on uniprocessor; would need cli/sti protection for SMP. */
static volatile uint32_t task_count = 0;
static uint32_t task_stacks[MAX_TASKS][TASK_STACK_WORDS] __attribute__((aligned(16)));

uint32_t task_create(void (*entry)(void)) {
    if (task_count >= MAX_TASKS) return (uint32_t)-1;
    uint32_t tid = task_count++;
    tasks[tid].id = tid;
    tasks[tid].state = TASK_READY;
    tasks[tid].stack = task_stacks[tid];
    uint32_t* stack_top = &task_stacks[tid][TASK_STACK_WORDS];
    stack_top--; *stack_top = 0x00000202;
    stack_top--; *stack_top = 0x08;
    stack_top--; *stack_top = (uint32_t)entry;
    for (int i = 0; i < 8; i++) { stack_top--; *stack_top = 0; }
    tasks[tid].context.esp = (uint32_t)stack_top;
    tasks[tid].context.eip = (uint32_t)entry;
    tasks[tid].context.eflags = 0x202;
    return tid;
}

/* task_switch must only be called after the PIC EOI has been sent.
 * The timer ISR path (irq_handler -> timer_handler -> task_switch) already does
 * this correctly. Calling task_switch before EOI would re-enable interrupts
 * (via popf) while the PIC still holds the IRQ line asserted, causing a
 * spurious interrupt storm. */
void task_switch(void) {
    uint32_t old_task = current_task;
    do {
        current_task = (current_task + 1) % task_count;
    } while (tasks[current_task].state != TASK_READY && current_task != old_task);
    if (current_task == old_task) return;
    /* pushf/popf save and restore the full EFLAGS including IF, so the
     * interrupt enable state of the caller is preserved across the switch
     * rather than being unconditionally turned on with sti. */
    __asm__ volatile(
        "pushf\n\t" "cli\n\t" "pusha\n\t"
        "mov %%esp, %0\n\t" "mov %1, %%esp\n\t"
        "popa\n\t" "popf\n\t"
        : "=m"(tasks[old_task].context.esp)
        : "m"(tasks[current_task].context.esp)
    );
}

void task_yield(void) { task_switch(); }
uint32_t task_get_current(void) { return current_task; }

void task_init(void) {
    task_count = 1;
    current_task = 0;
    tasks[0].id = 0;
    tasks[0].state = TASK_RUNNING;
    tasks[0].stack = NULL;
}
