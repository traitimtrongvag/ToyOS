#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED
} task_state_t;

void task_init(void);
uint32_t task_create(void (*entry)(void));
void task_switch(void);
void task_yield(void);
uint32_t task_get_current(void);

#endif
