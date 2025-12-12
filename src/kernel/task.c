/**
 * Task
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/task.h"

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
    uint32_t t_priority; // thread's priority
    uint32_t t_ticks; // maximum number of ticks
    uint32_t t_ticks_current; // current number of ticks
    uint32_t t_state; // task state
    uint32_t t_flags; // privilage level
    uint32_t t_page_dir; // paging directory
    int32_t t_exit_code; // exit code
    int32_t t_parent_pid;
    struct __thread_control_block *t_nextt; // next thread
    char const *t_name;
    struct __thread_fs *t_fs;
    FILE t_stdin;
    FILE t_stdout;
    FILE t_stderr;
};

extern FILE
    *stdin,
    *stdout,
    *stderr;

extern int32_t __alloc_buffer(FILE *file, char const *name, uint32_t size);

bool __sched_lock = FALSE;
struct __thread_control_block
    *thread_lhead = NULL;

struct __thread_control_block
    *thread_current = NULL;

struct __thread_control_block
    *thread_high_head = NULL,
    *thread_high_tail = NULL;

struct __thread_control_block
    *thread_low_head = NULL,
    *thread_low_tail = NULL;

static int32_t __thread_next_pid = 0;

FILE *__task_stdout(struct __thread_control_block *thread) {
    return &thread->t_stdout;
}

struct __thread_control_block *__task_next(struct __thread_control_block *thread) {
    return thread->t_nextt;
}

/**
 * __get_pid
*/

int32_t __get_pid(void) {
    if (!thread_current)
        return -1;

    return thread_current->t_pid;
}

/**
 * __get_dentry
*/

struct __dentry *__get_dentry(void) {
    if (!thread_current || !thread_current->t_fs)
        return NULL;

    return thread_current->t_fs->t_dentry;
}

/**
 * __get_state
*/

int32_t __get_state(int32_t pid, uint32_t *state) {
    __mutex_lock(&__sched_lock);
    struct __thread_control_block *thread = thread_high_head;

    while (thread) {
        if (thread->t_pid == pid) {
            *state = thread->t_state;
            __mutex_unlock(&__sched_lock);
            return 0;
        }

        thread = thread->t_nextt;

        if (thread == thread_high_head)
            break;
    }

    thread = thread_low_head;

    while (thread) {
        if (thread->t_pid == pid) {
            *state = thread->t_state;
            __mutex_unlock(&__sched_lock);
            return 0;
        }

        thread = thread->t_nextt;

        if (thread == thread_low_head)
            break;
    }

    __mutex_unlock(&__sched_lock);
    return -1;
}

/**
 * __get_exitcode
*/

int32_t __get_exitcode(int32_t pid, int32_t *exitcode) {
    __mutex_lock(&__sched_lock);
    struct __thread_control_block *thread = thread_high_head;

    while (thread) {
        if (thread->t_pid == pid) {
            *exitcode = thread->t_exit_code;
            __mutex_unlock(&__sched_lock);
            return 0;
        }

        thread = thread->t_nextt;

        if (thread == thread_high_head)
            break;
    }

    thread = thread_low_head;

    while (thread) {
        if (thread->t_pid == pid) {
            *exitcode = thread->t_exit_code;
            __mutex_unlock(&__sched_lock);
            return 0;
        }

        thread = thread->t_nextt;

        if (thread == thread_low_head)
            break;
    }

    __mutex_unlock(&__sched_lock);
    return -1;
}

/**
 * __exit
*/

__attribute__((noreturn)) int32_t __exit(int32_t code) {
    __mutex_lock(&__sched_lock);
    thread_current->t_state = THREAD_STATE_EXITING;
    thread_current->t_exit_code = code;
    __mutex_unlock(&__sched_lock);
    
    // idle loop
    for(;;)
        __yield();
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
    __exit(thread_current->t_exit_code);
}

/**
 * __create_thread
*/

extern FILE *stdin;

int32_t __create_thread(char const *name, int32_t (* main)(int argc, char **argv), uint32_t flags, uint32_t priority) {
    if (!thread_current)
        return -1;

    bool spawn_ring_0_thread = flags & THREAD_RING_0;

    // is thread allowed to spawn ring 0 thread?
    if (spawn_ring_0_thread && !(thread_current->t_flags & THREAD_RING_0))
        return -1;

    struct __thread_control_block *thread = (struct __thread_control_block *)kmalloc(sizeof(struct __thread_control_block));

    if (!thread)
        return -1;

    uint32_t *stack = (uint32_t *)pgalloc(PAGE_MAP);

    if (!stack) {
        kfree(thread);
        return -1;
    }

    uint32_t *kstack = (uint32_t *)pgalloc(PAGE_MAP);

    if (!kstack) {
        pgfree(stack);
        kfree(thread);
        return -1;
    }

    uint32_t const stdin_size = 512;

    char *stdin_base = (char *)kmalloc(stdin_size * sizeof(char));

    if (!stdin_base) {
        pgfree((void *)kstack);
        pgfree((void *)stack);
        kfree(thread);
        return -1;
    }

    thread->t_stdout.__lock = FALSE;

    if (__alloc_buffer(&thread->t_stdout, "stdout", 256)) {
        kfree(stdin);
        pgfree((void *)kstack);
        pgfree((void *)stack);
        kfree(thread);
        return -1;
    }

    // clear stack
    memset(stack, 0, 4096);
    memset(kstack, 0, 4096);

    thread->t_pid = __thread_next_pid++;
    thread->t_priority = priority;
    thread->t_state = THREAD_STATE_IDLE;
    thread->t_flags = flags;
    thread->t_page_dir = 0;
    thread->t_exit_code = -1;
    thread->t_parent_pid = thread_current->t_pid;
    thread->t_name = name;
    thread->t_fs = thread_current->t_fs;

    thread->t_stdin = (FILE){
        .__base = stdin_base,
        .__ptr = stdin_base,
        .__index = 0,
        .__count = 0,
        .__flags = 0,
        .__size = stdin_size,
        .__fname = "stdin",
        .__lock = FALSE
    };

    // all the data goes to terminal
    if (!strcmp(name, "terminal"))
        stdin = &thread->t_stdin;

    //thread->t_stderr = NULL;

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

    thread->t_ticks = 10;
    thread->t_ticks_current = 0;
    thread->t_nextt = NULL;
    
    // ----- CRITICAL -----
    __mutex_lock(&__sched_lock);
    switch (priority) {
        case THREAD_PRIORITY_LOW:
            if (!thread_low_head)
                thread_low_head = thread;

            if (thread_low_tail)
                thread_low_tail->t_nextt = thread;

            thread_low_tail = thread;
            break;

        case THREAD_PRIORITY_HIGH:
            if (!thread_high_head)
                thread_high_head = thread;

            if (thread_high_tail)
                thread_high_tail->t_nextt = thread;

            thread_high_tail = thread;
            break;
    }
    __mutex_unlock(&__sched_lock);
    // ----- CRITICAL -----
    return thread->t_pid;
}

/**
 * __sched_init
*/

int32_t __sched_init(struct __dentry *root_dentry) {
    struct __thread_control_block *kernel_thread = (struct __thread_control_block *)kmalloc(sizeof(struct __thread_control_block));
    
    if (!kernel_thread)
        return -1;

    struct __thread_fs *init_fs = (struct __thread_fs *)kmalloc(sizeof(struct __thread_fs));

    if (!init_fs) {
        kfree(kernel_thread);
        return -1;
    }

    uint32_t const stdin_size = 256;

    char *stdin_base = (char *)kmalloc(stdin_size * sizeof(char));

    if (!stdin_base) {
        kfree(init_fs);
        kfree(kernel_thread);
        return -1;
    }

    kernel_thread->t_stdout.__lock = FALSE;

    if (__alloc_buffer(&kernel_thread->t_stdout, "stdout", 256)) {
        kfree(stdin);
        kfree(init_fs);
        kfree(kernel_thread);
        return -1;
    }

    init_fs->t_dentry = root_dentry;

    kernel_thread->t_name = "kernel";
    kernel_thread->t_parent_pid = -1;
    kernel_thread->t_pid = __thread_next_pid++;
    kernel_thread->t_priority = THREAD_PRIORITY_HIGH;
    kernel_thread->t_state = THREAD_STATE_RUNNING;
    kernel_thread->t_flags = THREAD_RING_0;
    kernel_thread->t_exit_code = -1;
    kernel_thread->t_fs = init_fs;
    //kernel_thread->t_esp = 0;
    kernel_thread->t_stack = 0;
    // not used by thread 0
    kernel_thread->t_kernel_esp = 0;
    kernel_thread->t_kernel_stack = 0;
    kernel_thread->t_ticks = 5;
    kernel_thread->t_ticks_current = 0;
    
    kernel_thread->t_stdin = (FILE){
        .__base = stdin_base,
        .__ptr = stdin_base,
        .__index = 0,
        .__count = 0,
        .__flags = 0,
        .__size = stdin_size,
        .__fname = "stdin",
        .__lock = FALSE
    };
    
    asm volatile (
        "mov %0, cr3"
        : "=r" (kernel_thread->t_page_dir)
        :
        :
    );

    kernel_thread->t_nextt = NULL;

    __mutex_lock(&__sched_lock);
    thread_current = kernel_thread;
    thread_low_head = thread_low_tail = NULL;
    __mutex_unlock(&__sched_lock);
    return 0;
}

/**
 * __dispatch
 * 
 * TODO: ok, this needs a little refactoring to
 *  work correctly
*/

void __dispatch(void) {
    if (++thread_current->t_ticks_current < thread_current->t_ticks)
        return;

    // reset tick counter
    thread_current->t_ticks_current = 0;

    if (thread_current == THREAD_STATE_RUNNING)
        thread_current->t_state = THREAD_STATE_IDLE;

    switch (thread_current->t_priority) {
        case THREAD_PRIORITY_HIGH:
            if (!thread_high_head)
                thread_high_head = thread_current;

            if (thread_high_tail)
                thread_high_tail->t_nextt = thread_current;
            
            thread_high_tail = thread_current;
            break;
        
        case THREAD_PRIORITY_LOW:
            if (!thread_low_head)
                thread_low_head = thread_current;

            if (thread_low_tail)
                thread_low_tail->t_nextt = thread_current;
            
            thread_low_tail = thread_current;
            break;
    }

    struct __thread_control_block *next_thread;

    // skip blocked threads (waiting for an event (irq, mutex, etc.))
    // TODO: find first IDLE task
    if (thread_high_head/* && thread_high_head->t_state != THREAD_STATE_SLEEPING*/) {
        next_thread = thread_high_head;
        thread_high_head = thread_high_head->t_nextt;
    } else if (thread_low_head) {
        next_thread = thread_low_head;
        thread_low_head = thread_low_head->t_nextt;
    }

    thread_current = next_thread;
    stdout = &thread_current->t_stdout;
    stderr = &thread_current->t_stderr;
}

/**
 * __wake_on
*/

void __wake_on(bool *alarm) {
    __mutex_lock(&__sched_lock);
    thread_current->t_state = THREAD_STATE_SLEEPING;
    __mutex_unlock(&__sched_lock);

    // something could wake the thread up
    // so we check both the alarm and the state
    while (!*alarm/* && thread_current->t_state != THREAD_STATE_RUNNING*/)
        __yield();

    __mutex_lock(&__sched_lock);
    thread_current->t_state = THREAD_STATE_RUNNING;
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
    while (thread_current->t_state != THREAD_STATE_RUNNING);
    return 0;
}

/**
 * __sleep_me
*/

int32_t __sleep_me(void) {
    return __sleep_task(thread_current->t_pid);
}

/**
 * __wake_task
*/

int32_t __wake_task(int32_t pid) {
    __mutex_lock(&__sched_lock);
    struct __thread_control_block *thread = thread_current;
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
 * 
 * TODO: merge print of high and low tasks
 * 
 * ATTENTION: DO NOT USE THIS FUNCTION TO LIST
 *  TASKS, IT CAUSES DEAD-LOCK SINCE IT LOCKS
 *  THE SCHEDULER (i have to find another way
 *  to list tasks)
*/

void __list_threads(void) {
    __mutex_lock(&__sched_lock);
    
    struct __thread_control_block *thread = thread_low_head;
    
    if (thread)
        do {
            printk("thread pid=%u, state=%u, flags=%08x, prior=%u\n", thread->t_pid, thread->t_state, thread->t_flags, thread->t_priority);
            printk("  name: %s\n", thread->t_name);
            printk("  esp: %p, cr3: %p\n", thread->t_esp, thread->t_page_dir);
            thread = thread->t_nextt;
        } while (thread != thread_low_head);

    thread = thread_high_head;
    
    if (thread)
        do {
            printk("thread pid=%u, state=%u, flags=%08x, prior=%u\n", thread->t_pid, thread->t_state, thread->t_flags, thread->t_priority);
            printk("  name: %s\n", thread->t_name);
            printk("  esp: %p, cr3: %p\n", thread->t_esp, thread->t_page_dir);
            thread = thread->t_nextt;
        } while (thread != thread_high_head);
    
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