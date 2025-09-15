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
static bool mutex = FALSE;
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

int32_t __create_task(char const *name, uint32_t process, TASK_EXEC_MODE mode) {
    __mutex_lock(&mutex);

    if (!current_task) {
        __mutex_unlock(&mutex);
        return -1;
    }

    TASK *task = (TASK *)kmalloc(sizeof(TASK));
    
    if (!task) {
        __mutex_unlock(&mutex);
        return -1;
    }

    task->name = name;
    task->parent_pid = -1;
    task->pid = next_pid++;
    task->state = TASK_STATE_IDLE;
    task->mode = mode;
    task->code = -1;
    task->t_fs = current_task->t_fs;

    uint32_t stack = (uint32_t)/*pgalloc()*/e820_rmalloc(4096, TRUE) + 4096 - sizeof(uint32_t);

    if (!stack) {
        kfree(task);
        __mutex_unlock(&mutex);
        return -1;
    }

    *(uint32_t *)stack = (uint32_t)&__quiet_exit;

    task->esp = task->ebp = stack;
    task->kernel_stack = (uint32_t)/*pgalloc()*/e820_rmalloc(4096, TRUE) + 4096;

    if (!task->kernel_stack) {
        kfree(task);
        pgfree((void *)stack);
        __mutex_unlock(&mutex);
        return -1;
    }

    task->eip = process;
    task->next = first_task;
    last_task->next = task;
    last_task = task;
    
    __mutex_unlock(&mutex);
    return task->pid;
}

/**
 * __sleep_task
*/

int32_t __sleep_task(int32_t pid) {
    __mutex_lock(&mutex);
    TASK *task = first_task;
    bool not_found = TRUE;

    do {
        if (task->pid == pid) {
            not_found = FALSE;
            break;
        }

        task = task->next;
    } while (task != first_task);

    if (not_found) {
        __mutex_unlock(&mutex);
        return -1;
    }

    task->state = TASK_STATE_SLEEP;
    __mutex_unlock(&mutex);
    // wait for task switch, TODO: invoke task switch?
    while (current_task->state != TASK_STATE_RUNNING);
    return 0;
}

/**
 * __sleep_me
*/

int32_t __sleep_me(void) {
    return __sleep_task(current_task->pid);
}

/**
 * __wake_task
*/

int32_t __wake_task(int32_t pid) {
    __mutex_lock(&mutex);
    TASK *task = first_task;
    bool not_found = TRUE;

    do {
        if (task->pid == pid) {
            not_found = FALSE;
            break;
        }

        task = task->next;
    } while (task != first_task);

    if (not_found) {
        __mutex_unlock(&mutex);
        return -1;
    }

    if (task->state != TASK_STATE_SLEEP) {
        __mutex_unlock(&mutex);
        return -2;
    }

    task->state = TASK_STATE_WAKE;
    __mutex_unlock(&mutex);
    return 0;
}

/**
 * __wake_on
*/

/*int32_t __wake_on(bool *wake_on) {
    current_task->wake_on = wake_on;
    return 0;
}*/

/**
 * __init_tasking
*/

int32_t __init_tasking(void) {
    __mutex_lock(&mutex);
    printk("Initializing tasking... ");

    first_task = (TASK *)kmalloc(sizeof(TASK));

    if (!first_task) {
        __mutex_unlock(&mutex);
        return -1;
    }

    //struct __task_fs *fs = (struct __task_fs *)kmalloc(sizeof(struct __task_fs));

    /*if (!fs) {
        __mutex_unlock(&mutex);
        return -2;
    }

    fs->t_users = 1;
    fs->t_dentry = NULL; // no root mounted*/

    first_task->name = "kernel";
    first_task->parent_pid = -1;
    first_task->pid = next_pid++;
    first_task->state = TASK_STATE_RUNNING;
    first_task->mode = TASK_EXEC_KERNEL;
    first_task->code = -1;
    //first_task->t_fs = fs;

    void *stack = e820_rmalloc(4096, TRUE); //pgalloc();

    if (!stack) {
        kfree(first_task);
        __mutex_unlock(&mutex);
        return -2;
    }

    first_task->esp = first_task->ebp = (uint32_t)stack + 4096;

    void *kstack = e820_rmalloc(4096, TRUE); //pgalloc();

    if (!kstack) {
        pgfree(stack);
        kfree(first_task);
        __mutex_unlock(&mutex);
        return -3;
    }

    first_task->kernel_stack = (uint32_t)kstack + 4096;
    first_task->next = first_task;
    last_task = current_task = first_task;

    printf("Ok\n");
    __mutex_unlock(&mutex);
    return 0;
}

/**
 * __list_tasks
*/

void __list_tasks(void) {
    __mutex_lock(&mutex);
    
    TASK *task = first_task;
    
    do {
        printk("task pid=%u, state=%u, mode=%u, name=%s\n", task->pid, task->state, task->mode, task->name);
        task = task->next;
    } while (task != first_task);
    
    __mutex_unlock(&mutex);
}

/**
 * __switch_task
 * 
 * FIXME:
 *  switch task should lock mutex!!!
*/

void __switch_task(void) {
    if (!current_task)
        return; // not initialized

    if (__test_set(&mutex))
        return;

    /*if (current_task == current_task->next)
        return; // only one task*/

    // idle current task
    if (current_task->state == TASK_STATE_RUNNING)
        current_task->state = TASK_STATE_IDLE;

    /**
     * two things can happen here:
     * 1) __read_eip returned the current processes address and
     *    we're going to do task switch
     * 2) the task switch just happened and the old process is
     *    being resumed
    */

    uint32_t eip = __read_eip(); // get current eip

    // if the task state is TASK_STATE_RUNNING, task switch happend and
    // we're being resumed, otherwise we're performing the task switch
    if (current_task->state == TASK_STATE_RUNNING)
        return; // resumed task, let it run

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
    bool get_next_task;
    
    do {
        get_next_task = FALSE;

        if (next_task->state == TASK_STATE_EXITING) {
            printk("task %u exited with code %u\n", next_task->pid, next_task->code);

            current_task->next = next_task->next;
            pgfree((void *)next_task->esp);
            pgfree((void *)next_task->kernel_stack);
            kfree(next_task);

            next_task = next_task->next;
            get_next_task = TRUE;
        } else if (next_task->state == TASK_STATE_SLEEP) {
            /*if (next_task->wake_on && *next_task->wake_on)
                next_task->wake_on = NULL;
            else {*/
                //next_task->state = TASK_STATE_SLEEPING;
                next_task = next_task->next;
                get_next_task = TRUE;
            //}
        } /*else if (next_task->state == TASK_STATE_SLEEPING) {
            next_task = next_task->next;
            get_next_task = TRUE;
        }*/
    } while (get_next_task);

    // idle, wake and running tasks can pass
    current_task = next_task;
    current_task->state = TASK_STATE_RUNNING;

    //printk("switching to task=%p, state=%u\n", current_task->eip, current_task->state);

    //__set_kernel_stack(current_task->kernel_stack); // associated kernel stack

    /**
     * ATTENTION:
     *  if a new task is created, switch task jumps
     *  right into its code, so we must unlock the
     *  mutex here (in __exec_*mode, it is safer)
    */
    //__mutex_unlock(&mutex);

    //if (current_task->mode)
        __exec_kernelmode(current_task->eip, current_task->esp, current_task->ebp, &mutex);
    //else
    //    __exec_usermode(current_task->eip, current_task->esp, current_task->ebp);
}

/**
 * fork
*/

int32_t fork(void) {
    __disable_interrupts();

    TASK *parent_task = current_task;
    TASK *task = (TASK *)kmalloc(sizeof(TASK));
    task->parent_pid = parent_task->pid;
    task->pid = next_pid;
    task->state = TASK_STATE_IDLE;
    task->mode = TASK_EXEC_USER;
    task->kernel_stack = (uint32_t)pgalloc() + 4096;
    task->next = first_task;
    last_task->next = task;
    last_task = task;

    uint32_t eip = __read_eip();

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