/**
 * Task
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/task.h"
#include "drivers/vga.h"

#define THREAD_STATE_IDLE 0
#define THREAD_STATE_RUNNING 1
#define THREAD_STATE_SLEEPING 2
#define THREAD_STATE_EXITING 3

#define push_32(stack, value) *--stack = (value)

struct __thread_fs {
    //uint32_t t_users;
    struct __dentry *t_dentry;
};

struct __thread_control_block {
    uint32_t t_esp; // current esp
    uint32_t t_stack; // stack bottom
    uint32_t t_kernel_esp; // current kernel esp
    uint32_t t_kernel_stack; // kernel stack bottom
    int32_t t_pid; // process id
    uint32_t t_state; // task state
    uint32_t t_flags; // privilage level
    uint32_t t_page_dir; // paging directory
    int32_t t_exit_code; // exit code
    int32_t t_parent_pid;
    struct __thread_control_block *t_nextt; // next thread
    char const *t_name;
    struct __thread_fs *t_fs;
    char *strtok_sptr; // strtok string pointer
    /*FILE *t_stdin;
    FILE *t_stdout;
    FILE *t_stderr;*/
};

bool __sched_lock = FALSE;
struct __thread_control_block
    *thread_lhead = NULL,
    *thread_lcurrent = NULL,
    *thread_ltail = NULL;

static int32_t __thread_next_pid = 0;

/**
 * __get_pid
*/

int32_t __get_pid(void) {
    if (!thread_lcurrent)
        return -1;

    return thread_lcurrent->t_pid;
}

/**
 * __get_dentry
*/

struct __dentry *__get_dentry(void) {
    if (!thread_lcurrent || !thread_lcurrent->t_fs)
        return NULL;

    return thread_lcurrent->t_fs->t_dentry;
}

/**
 * __exit
*/

__attribute__((noreturn)) int32_t __exit(int32_t code) {
    __mutex_lock(&__sched_lock);
    thread_lcurrent->t_state = THREAD_STATE_EXITING;
    thread_lcurrent->t_exit_code = code;
    __mutex_unlock(&__sched_lock);
    
    for(;;); // idle loop
}

/**
 * __thread_setup
*/

static void __thread_setup(void) {
    // returns into main
}

/**
 * __thread_exit
*/

static void __thread_exit(void) {
    __exit(thread_lcurrent->t_exit_code);
}

/**
 * __create_thread
*/

int32_t __create_thread(char const *name, int32_t (* main)(int argc, char **argv), uint32_t flags) {
    if (!thread_lcurrent)
        return -1;

    // 
    bool spawn_ring_0_thread = flags & THREAD_RING_0;

    // is thread allowed to spawn ring 0 thread?
    if (spawn_ring_0_thread && !(thread_lcurrent->t_flags & THREAD_RING_0))
        return -2;

    //printk("%u\n", sizeof(struct __thread_control_block));
    kmalloc(63UL);

    if (__thread_next_pid++ == 1)
        return 1;

    struct __thread_control_block *thread = (struct __thread_control_block *)kmalloc(sizeof(struct __thread_control_block));

    if (!thread)
        return -3;

    uint32_t *stack = (uint32_t *)pgalloc();

    if (!stack) {
        kfree(thread);
        return -4;
    }

    uint32_t *kstack = (uint32_t *)pgalloc();

    if (!kstack) {
        pgfree(stack);
        kfree(thread);
        return -5;
    }

    // clear stack
    memset(stack, 0, 4096);
    memset(kstack, 0, 4096);

    thread->t_pid = __thread_next_pid++;
    thread->t_state = THREAD_STATE_IDLE;
    thread->t_flags = flags;
    thread->t_page_dir = 0;
    thread->t_exit_code = -1;
    thread->t_parent_pid = thread_lcurrent->t_pid;

    thread->t_name = name;
    thread->t_fs = thread_lcurrent->t_fs;
    /*thread->t_stdin = NULL;
    thread->t_stdout = NULL;
    thread->t_stderr = NULL;*/

    // set stack bottom for pgfree
    thread->t_stack = (uint32_t)stack;
    thread->t_kernel_stack = (uint32_t)kstack;

    // += (KERNEL_)STACK_SIZE / sizeof(uint32_t)
    stack += 1024;
    kstack += 1024;

    *--stack = (uint32_t)&__thread_exit; // thread exit
    *--stack = (uint32_t)main; // eip

    // set up thread stack
    if (!spawn_ring_0_thread) {
        uint32_t temp = (uint32_t)stack;
        *--stack = 0x00000023;
        *--stack = temp;
    }

    *--stack = 0x00000202; // eflags

    if (spawn_ring_0_thread)
        *--stack = 0x00000008; // cs
    else
        *--stack = 0x0000001b; // cs
    
    *--stack = (uint32_t)&__thread_setup; // thread setup
    
    uint32_t temp = (uint32_t)stack;

    *--stack = 0x00000000;
    *--stack = 0x00000000;
    *--stack = 0x00000000;
    *--stack = 0x00000000;
    *--stack = temp;
    *--stack = 0x00000000;
    *--stack = 0x00000000;
    *--stack = 0x00000000;

    if (spawn_ring_0_thread) {
        *--stack = 0x00000010;
        *--stack = 0x00000010;
        *--stack = 0x00000010;
        *--stack = 0x00000010;
    } else {
        *--stack = 0x00000023;
        *--stack = 0x00000023;
        *--stack = 0x00000023;
        *--stack = 0x00000023;
    }

    thread->t_esp = (uint32_t)stack;

    // set up kernel stack
    thread->t_kernel_esp = (uint32_t)kstack;
    
    // ----- CRITICAL -----
    __mutex_lock(&__sched_lock);
    thread->t_nextt = thread_lhead;
    thread_ltail->t_nextt = thread;
    thread_ltail = thread;
    __mutex_unlock(&__sched_lock);
    // ----- CRITICAL -----
    return thread->t_pid;
}

/**
 * __sched_init
*/

int32_t __sched_init(struct __dentry *root_dentry) {
    struct __thread_control_block *init = (struct __thread_control_block *)kmalloc(sizeof(struct __thread_control_block));
    
    if (!init)
        return -1;

    struct __thread_fs *init_fs = (struct __thread_fs *)kmalloc(sizeof(struct __thread_fs));

    if (!init_fs) {
        kfree(init);
        return -1;
    }

    init_fs->t_dentry = root_dentry;

    init->t_name = "kernel";
    init->t_parent_pid = -1;
    init->t_pid = __thread_next_pid++;
    init->t_state = THREAD_STATE_RUNNING;
    init->t_flags = THREAD_RING_0;
    init->t_exit_code = -1;
    init->t_fs = init_fs;
    //init->t_esp = 0;
    init->t_stack = 0;
    // not used by thread 0
    init->t_kernel_esp = 0;
    init->t_kernel_stack = 0;
    
    asm volatile (
        "mov %0, cr3"
        : "=r" (init->t_page_dir)
        :
        :
    );

    init->t_nextt = init;

    __mutex_lock(&__sched_lock);
    thread_lhead = thread_lcurrent = thread_ltail = init;
    __mutex_unlock(&__sched_lock);
    return 0;
}

/**
 * __dispatch
*/

void __dispatch(void) {
    if (thread_lcurrent->t_state == THREAD_STATE_RUNNING)
        thread_lcurrent->t_state = THREAD_STATE_IDLE;

    struct __thread_control_block *prev_thread;
    struct __thread_control_block *next_thread = thread_lcurrent;
    bool next;

    do {
        prev_thread = next_thread;
        next_thread = next_thread->t_nextt;
        next = FALSE;

        switch (next_thread->t_state) {
            case THREAD_STATE_SLEEPING:
                next = TRUE;
                break;

            case THREAD_STATE_EXITING:
                if (next_thread->t_stack)
                    pgfree((void *)next_thread->t_stack);

                if (next_thread->t_kernel_stack)
                    pgfree((void *)next_thread->t_kernel_stack);

                prev_thread->t_nextt = next_thread->t_nextt;

                if (next_thread == thread_lhead)
                    thread_lhead = next_thread->t_nextt;

                if (next_thread == thread_ltail)
                    thread_ltail = next_thread->t_nextt;

                kfree(next_thread);
                next_thread = prev_thread;
                next = TRUE;
                break;
        }
    } while (next);

    thread_lcurrent = next_thread;
    thread_lcurrent->t_state = THREAD_STATE_RUNNING;
    return;
}

/**
 * __wake_on
*/

void __wake_on(bool *alarm) {
    //__mutex_lock(&__sched_lock);
    //thread_lcurrent->t_state = THREAD_STATE_SLEEPING;
    //__mutex_unlock(&__sched_lock);

    // something could wake the thread up
    // so we check both the alarm and the state
    while (!*alarm/* && thread_lcurrent->t_state != THREAD_STATE_RUNNING*/);

    __mutex_lock(&__sched_lock);
    thread_lcurrent->t_state = THREAD_STATE_RUNNING;
    __mutex_unlock(&__sched_lock);
}

/**
 * __sleep_task
*/

int32_t __sleep_task(int32_t pid) {
    __mutex_lock(&__sched_lock);
    struct __thread_control_block *thread = thread_lhead;
    bool not_found = TRUE;

    // TODO: compare current thread pid with pid
    do {
        if (thread->t_pid == pid) {
            not_found = FALSE;
            break;
        }

        thread = thread->t_nextt;
    } while (thread != thread_lhead);

    if (not_found) {
        __mutex_unlock(&__sched_lock);
        return -1;
    }

    thread->t_state = THREAD_STATE_SLEEPING;
    __mutex_unlock(&__sched_lock);
    // wait for ctx switch, TODO: invoke ctx switch?

    // if the thread is the current thread
    while (thread_lcurrent->t_state != THREAD_STATE_RUNNING);
    return 0;
}

/**
 * __sleep_me
*/

int32_t __sleep_me(void) {
    return __sleep_task(thread_lcurrent->t_pid);
}

/**
 * __wake_task
*/

int32_t __wake_task(int32_t pid) {
    __mutex_lock(&__sched_lock);
    struct __thread_control_block *thread = thread_lcurrent;
    bool not_found = TRUE;

    do {
        if (thread->t_pid == pid) {
            not_found = FALSE;
            break;
        }

        thread = thread->t_nextt;
    } while (thread != thread_lhead);

    if (not_found) {
        __mutex_unlock(&__sched_lock);
        return -1;
    }

    if (thread->t_state != THREAD_STATE_SLEEPING) {
        __mutex_unlock(&__sched_lock);
        return 0; // thread is awake, return error?
    }

    thread->t_state = THREAD_STATE_IDLE;
    __mutex_unlock(&__sched_lock);
    return 0;
}

/**
 * __list_threads
*/

void __list_threads(void) {
    __mutex_lock(&__sched_lock);
    
    struct __thread_control_block *thread = thread_lhead;
    
    do {
        printk("thread pid=%u, state=%u, flags=%08x, name=%s\n", thread->t_pid, thread->t_state, thread->t_flags, thread->t_name);
        printk("  esp: %p, cr3: %p\n", thread->t_esp, thread->t_page_dir);
        thread = thread->t_nextt;
    } while (thread != thread_lhead);
    
    __mutex_unlock(&__sched_lock);
}

/**
 * fork
*/

/*int32_t fork(void) {
    __disable_interrupts();

    TASK *parent_task = current_task;
    TASK *task = (TASK *)kmalloc(sizeof(TASK));
    task->parent_pid = parent_task->pid;
    task->pid = next_pid;
    task->state = TASK_STATE_IDLE;
    task->mode = TASK_EXEC_USER;
    task->next = first_task;
    last_task->next = task;
    last_task = task;

    uint32_t eip = __read_eip();

    **
     * the `parent_task' will be always the same
     * (vas) as replicated but the `current_task'
     * changes (is `parent_task' or `task') 
    *

    if (parent_task == current_task) {
        uint32_t esp, ebp;
        
        asm volatile (
            "mov %0, esp\n\t"
            "mov %1, ebp"
            : "=r" (esp), "=r" (ebp)
            :
            :
        );
        
        task->esp = esp;
        task->ebp = ebp;

        __enable_interrupts();
        return next_pid++;
    }

    return 0;
}*/