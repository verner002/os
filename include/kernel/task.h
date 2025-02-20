/**
 * Task
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"
#include "drivers/cpu.h"
#include "kernel/pager.h"
#include "kernel/userland.h"
#include "kstdlib/stdlib.h"

#include "kstdlib/stdio.h"

/**
 * Types Definitions
*/

typedef enum __task_state TASK_STATE;
typedef struct __task TASK;

/**
 * Enumerations
*/

enum __task_state {
    TASK_STATE_IDLE = 0,
    TASK_STATE_RUNNING = 1,
    TASK_STATE_EXITING = 2
};

/**
 * Structures
*/

struct __task {
    int32_t parent_pid;
    int32_t pid;
    TASK_STATE state;
    int32_t code;
    uint32_t eip;
    uint32_t esp;
    uint32_t ebp;
    uint32_t paging_directory;
    uint32_t kernel_stack;
    TASK *next;
};

/**
 * Declarations
*/

int32_t __create_task(uint32_t process);
int32_t __get_pid(void);
int32_t __exit(int32_t code);
void __list_tasks(void);
int32_t __init_tasking(void);
void __switch_task(void);
int32_t fork(void);