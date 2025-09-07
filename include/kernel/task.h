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
#include "kernel/ts.h"
#include "kernel/heap.h"
#include "kernel/mutex.h"
#include "hal/vfs.h"
#include "kstdlib/stdio.h"

/**
 * Types Definitions
*/

typedef enum __task_state TASK_STATE;
typedef enum __task_exec_mode TASK_EXEC_MODE;
typedef struct __task TASK;

/**
 * Enumerations
*/

enum __task_state {
    TASK_STATE_IDLE = 0,
    TASK_STATE_RUNNING = 1,
    TASK_STATE_EXITING = 2,
    TASK_STATE_SLEEP = 3,
    TASK_STATE_SLEEPING = 4,
    TASK_STATE_WAKE = 5
};

enum __task_exec_mode {
    TASK_EXEC_USER = 0,
    TASK_EXEC_KERNEL = 1
};

/**
 * Structures
*/

struct __task_fs {
    uint32_t t_users;
    struct __dentry *t_dentry;
};

struct __task {
    int32_t parent_pid;
    char const *name;
    int32_t pid;
    TASK_STATE state;
    TASK_EXEC_MODE mode;
    int32_t code;
    uint32_t eip;
    uint32_t esp;
    uint32_t ebp;
    uint32_t paging_directory;
    struct __task_fs *t_fs;
    /*FILE *stdin;
    FILE *stdout;
    FILE *stderr;*/
    uint32_t kernel_stack;
    TASK *next;
};

/**
 * Declarations
*/

int32_t __create_task(char const *name, uint32_t process, TASK_EXEC_MODE mode);
int32_t __sleep_task(int32_t pid);
int32_t __sleep_me(void);
int32_t __wake_task(int32_t pid);
int32_t __get_pid(void);
int32_t __exit(int32_t code);
void __list_tasks(void);
int32_t __init_tasking(void);
void __switch_task(void);
int32_t fork(void);