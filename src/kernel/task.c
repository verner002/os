/**
 * Task
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/task.h"

/**
 * Static Global Variables
*/

static int32_t next_pid = 0;
static TASK
    *first_task,
    *last_task,
    *current_task = NULL;

/**
 * __get_pid
*/

int32_t __get_pid(void) {
    return current_task->pid;
}

/**
 * __exit
*/

int32_t __exit(int32_t code) {
    current_task->state = TASK_STATE_EXITING;
    current_task->code = code;
    for(;;); // idle loop
}

/**
 * __quiet_exit
*/

void __quiet_exit(void) {
    __exit(0);
}

/**
 * __create_task
*/

int32_t __create_task(uint32_t process) {
    __disable_interrupts();

    TASK *task = (TASK *)malloc(sizeof(TASK));
    task->parent_pid = -1;
    task->pid = next_pid++;
    task->state = TASK_STATE_IDLE;
    task->code = -1;

    uint32_t stack = (uint32_t)pgalloc() + 4096 - sizeof(uint32_t);
    *(uint32_t *)stack = (uint32_t)&__quiet_exit;

    task->esp = task->ebp = stack;
    task->kernel_stack = (uint32_t)pgalloc() + 4096;
    task->eip = process;
    task->next = first_task;
    last_task->next = task;
    last_task = task;
    
    __enable_interrupts();
    return task->pid;
}

/**
 * __init_tasking
*/

int32_t __init_tasking(void) {
    printk("Initializing tasking... ");

    __disable_interrupts();

    first_task = last_task = current_task = (TASK *)malloc(sizeof(TASK));
    first_task->parent_pid = -1;
    first_task->pid = next_pid++;
    first_task->state = TASK_STATE_RUNNING;
    first_task->code = -1;
    first_task->esp = first_task->ebp = (uint32_t)pgalloc() + 4096;
    first_task->kernel_stack = (uint32_t)pgalloc() + 4096;
    first_task->next = first_task;

    __enable_interrupts();

    printf("Ok\n");
    return 0;
}

/**
 * __list_tasks
*/

void __list_tasks(void) {
    //__disable_interrupts();
    
    TASK *task = first_task;
    
    do {
        printk("task pid=%u, state=%u\n", task->pid, task->state);
        task = task->next;
    } while (task != first_task);
    
    //__enable_interrupts();
}

/**
 * __switch_task
*/

void __switch_task(void) {
    if (!current_task) return; // not initialized

    //if (current_task == current_task->next) return; // only one task

    // idle current task
    if (current_task->state == TASK_STATE_RUNNING)
        current_task->state = TASK_STATE_IDLE;

    /**
     * two things can happen here:
     * 1) __get_eip returned the current processes address and we're going to do task switch
     * 2) the task switch just happened and the old process is being resumed
    */

    uint32_t eip = __get_eip(); // get current eip

    // if the task state is TASK_STATE_RUNNING, task switch happend and
    // we're being resumed, otherwise we're performing the task switch
    if (current_task->state == TASK_STATE_RUNNING) return; // resumed task

    uint32_t esp, ebp;

    asm (
        "mov %0, esp\t\n"
        "mov %1, ebp"
        : "=m" (esp), "=m" (ebp)
        :
        :
    );
    
    // store current task state
    current_task->eip = eip;
    current_task->esp = esp;
    current_task->ebp = ebp;

    // move to next task
    TASK *next_task = current_task->next;

    if (next_task->state == TASK_STATE_EXITING) {
        printk("task %u exited with code %u\n", next_task->pid, next_task->code);

        current_task->next = next_task->next;
        pgfree((void *)next_task->esp);
        pgfree((void *)next_task->kernel_stack);
        free(next_task);
    }

    current_task = current_task->next;
    current_task->state = TASK_STATE_RUNNING;

    //printk("switching to task=%p, state=%u\n", current_task->eip, current_task->state);

    //__set_kernel_stack(current_task->kernel_stack); // associated kernel stack

    __exec_kernelmode(current_task->eip, current_task->esp, current_task->ebp);
}

/**
 * fork
*/

int32_t fork(void) {
    __disable_interrupts();

    TASK *parent_task = current_task;
    TASK *task = (TASK *)malloc(sizeof(TASK));
    task->parent_pid = parent_task->pid;
    task->pid = next_pid;
    task->state = TASK_STATE_IDLE;
    task->kernel_stack = (uint32_t)pgalloc() + 4096;
    task->next = first_task;
    last_task->next = task;
    last_task = task;

    uint32_t eip = __get_eip();

    /**
     * the `parent_task' will be always the same
     * (vas) as replicated but the `current_task'
     * changes (is `parent_task' or `task') 
    */

    if (parent_task == current_task) {
        uint32_t esp, ebp;
        
        asm volatile (
            "mov %0, esp\t\n"
            "mov %1, ebp"
            : "=r" (esp), "=r" (ebp)
            :
            :
        );
        
        task->esp = esp;
        task->ebp = ebp;
        task->eip = eip;

        __enable_interrupts();
        return next_pid++;
    }

    return 0;
}