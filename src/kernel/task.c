/**
 * @file task.c
 * @author verner002
 * @date 08/02/2026
*/

#include "macros.h"
#include "kernel/atomic.h"
#include "kernel/task.h"
#include "kernel/panic.h"

#define THREAD_STATE_IDLE 0
#define THREAD_STATE_RUNNING 1
#define THREAD_STATE_SLEEPING 2
#define THREAD_STATE_EXITING 3

#define push_32(stack, value) *--stack = (value)

struct thread_fs {
    //uint32_t t_users;
    struct dentry *t_dentry;
};

struct __thread_control_block {
    uint32_t t_esp; // current esp
    uint32_t t_stack; // stack bottom
    uint32_t t_kernel_esp; // current kernel esp
    uint32_t t_kernel_stack; // kernel stack bottom
    uint32_t t_page_dir; // paging directory
    int32_t t_pid; // process id
    uint32_t t_priority; // thread's priority
    uint32_t t_ticks; // maximum number of ticks
    uint32_t t_ticks_current; // current number of ticks
    uint32_t t_state; // task state
    uint32_t t_flags; // privilage level
    int t_exit_code; // exit code
    int32_t t_parent_pid;
    struct __thread_control_block *t_nextt; // next thread
    char const *t_name;
    struct thread_fs *t_fs;
    FILE t_stdin;
    FILE t_stdout;
    FILE t_stderr;
    void (*on_exit)(int exit_code);
};

extern FILE
    *stdin,
    *stdout,
    *stderr;

// function for std-buffer allocation (stdin, stdout, etc.)
extern void thread_exit(void);
extern int32_t __alloc_buffer(FILE *file, char const *name, uint32_t size);

bool __sched_lock = false;

// it's not a good idea to use
// circular linked-list since
// there is a posibility that
// something may deallocate
// and then allocate memory
// used by one of the tcbs and
// that would make an initial
// pointer invalid
struct __thread_control_block
    *thread_lhead = NULL,
    *thread_current = NULL,
    *thread_ltail = NULL;

// TODO: use atomic_t for this one
// used to assign each task a different id
static atomic_t __thread_next_pid = 0;

/**
 * __get_pid
*/

int32_t __get_pid(void) {
    if (unlikely(!thread_current))
        panic(); // we don't know what to do

    return thread_current->t_pid;
}

/**
 * __get_dentry
*/

struct dentry *current_dentry(void) {
    if (unlikely(!thread_current || !thread_current->t_fs))
        panic();

    return thread_current->t_fs->t_dentry;
}

/**
 * __get_state
 * 
 * NOTE: ONLY FOR TESTING PURPOSES
*/

int32_t __get_state(int32_t pid, uint32_t *state) {
    if (!state)
        return -1; // return different code?

    __mutex_lock(&__sched_lock);
    struct __thread_control_block *thread = thread_lhead;

    if (unlikely(!thread)) {
        // not necessary but for clearness
        __mutex_unlock(&__sched_lock);
        panic();
    }

    do {
        if (thread->t_pid == pid) {
            *state = thread->t_state;
            __mutex_unlock(&__sched_lock);
            return 0;
        }

        thread = thread->t_nextt;
    } while (thread);

    __mutex_unlock(&__sched_lock);
    return -1; // not found
}

/**
 * __get_exitcode
 * 
 * TODO: let tasks sign for an exit event and store
 *  the exit code at each of the given addresses (this
 *  is safer because tasks themself are responsible for
 *  the allocated memory but it is surely slower)
*/

int32_t __get_exitcode(int32_t pid, int32_t *exitcode) {
    __mutex_lock(&__sched_lock);
    struct __thread_control_block *thread = thread_lhead;

    if (unlikely(!thread)) {
        __mutex_unlock(&__sched_lock);
        panic();
    }

    do {
        if (thread->t_pid == pid) {
            *exitcode = thread->t_exit_code;
            __mutex_unlock(&__sched_lock);
            return 0;
        }

        thread = thread->t_nextt;
    } while (thread);

    __mutex_unlock(&__sched_lock);
    return -1; // not found
}

/**
 * __exit
*/

__attribute__((noreturn)) int32_t __exit(int32_t code) {
    __mutex_lock(&__sched_lock);
    thread_current->t_state = THREAD_STATE_EXITING;
    thread_current->t_exit_code = code;
    __mutex_unlock(&__sched_lock);
    
    // we loop forever and wait for scheduler to
    // remove us from the queue
    // in case we don't want to just loop during
    // the given time we can just tell scheduler
    // that we're done
    for(;;)
        __yield();
}

/**
 * __thread_setup
*/

static void __thread_setup(void) {
    // returns into main
    // common setup function for all threads
}

/**
 * __thread_exit
*/

static void __thread_exit(void) {
    // common exit function for all threads
    __exit(thread_current->t_exit_code);
}

/**
 * __sched_init
*/

int32_t __sched_init(struct dentry *root_dentry) {
    struct __thread_control_block *kernel_thread = (struct __thread_control_block *)kmalloc(sizeof(struct __thread_control_block));
    
    if (!kernel_thread)
        return -1;

    struct thread_fs *init_fs = (struct thread_fs *)kmalloc(sizeof(struct thread_fs));

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

    kernel_thread->t_stdout.__lock = false;

    if (__alloc_buffer(&kernel_thread->t_stdout, "stdout", 4096)) {
        kfree(stdin_base);
        kfree(init_fs);
        kfree(kernel_thread);
        return -1;
    }

    init_fs->t_dentry = root_dentry;

    kernel_thread->t_name = "kernel";
    kernel_thread->t_parent_pid = -1;
    kernel_thread->t_pid = atomic_fetch_add(&__thread_next_pid, 1);
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
        .__lock = false
    };
    
    asm volatile (
        "mov %0, cr3"
        : "=r" (kernel_thread->t_page_dir)
        :
        :
    );

    kernel_thread->t_nextt = NULL;

    __mutex_lock(&__sched_lock);
    // initialize scheduler linked-list
    thread_lhead = thread_current = thread_ltail = kernel_thread;
    __mutex_unlock(&__sched_lock);
    return 0;
}

/**
 * __create_thread
*/

int32_t __create_thread(char const *name, int32_t (* main)(int argc, char **argv), uint32_t flags, uint32_t priority, void (*on_exit_handler)(int exit_code)) {
    if (unlikely(!thread_lhead || !thread_current || !thread_ltail))
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

    void *pg_dir = pgalloc(PAGE_MAP);

    if (!pg_dir) {
        kfree(stdin_base);
        pgfree((void *)kstack);
        pgfree((void *)stack);
        kfree(thread);
        return -1;
    }

    thread->t_stdout.__lock = false;

    if (__alloc_buffer(&thread->t_stdout, "stdout", 4096)) {
        kfree(pg_dir);
        kfree(stdin_base);
        pgfree((void *)kstack);
        pgfree((void *)stack);
        kfree(thread);
        return -1;
    }

    // clear stack
    memset(stack, 0, 4096);
    memset(kstack, 0, 4096);

    // copy kernel area mapping
    memcpy(pg_dir, (void *)(thread_lhead->t_page_dir & 0xfffff000), 1024);

    thread->t_pid = atomic_fetch_add(&__thread_next_pid, 1);
    thread->t_priority = priority;
    thread->t_state = THREAD_STATE_IDLE;
    thread->t_flags = flags;
    thread->t_page_dir = (uint32_t)pg_dir | (uint32_t)(thread_lhead->t_page_dir & 0xfff);
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
        .__lock = false
    };

    thread->on_exit = on_exit_handler;

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

    *--stack = (uint32_t)&thread_exit; // thread exit
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
    // tell current tail that we're joining
    thread_ltail->t_nextt = thread;
    // and also taking its place
    thread_ltail = thread;
    __mutex_unlock(&__sched_lock);
    // ----- CRITICAL -----
    return thread->t_pid;
}

/**
 * __dispatch
*/

void __dispatch(void) {
    // we're still enjoying our time
   /* if (++thread_current->t_ticks_current < thread_current->t_ticks)
        return;*/

    // reset tick counter
    thread_current->t_ticks_current = 0;

    // perform transition from RUNNING to IDLE
    // FIXME: use SLEEPING instead of IDLE
    if (thread_current->t_state == THREAD_STATE_RUNNING)
        thread_current->t_state = THREAD_STATE_IDLE;

    // TODO: skip blocked threads (waiting
    //  for an event (irq, mutex, etc.))

    do {
        // starting with thread_thead would
        // result in prioritizing tasks at
        // the beginning of the list
        thread_current = thread_current->t_nextt;

        if (!thread_current)
            thread_current = thread_lhead;

        if (thread_current->t_state == THREAD_STATE_EXITING) {
            // free the memory occupied by the task
            // and return error code to subscribes
            if (thread_current->on_exit)
                thread_current->on_exit(thread_current->t_exit_code);

            thread_current->t_state = -1; // temp solution
        }
    } while (likely(
        thread_current->t_state != THREAD_STATE_IDLE &&
        thread_current->t_state != THREAD_STATE_SLEEPING
    ));

    if (thread_current->t_state != THREAD_STATE_SLEEPING)
        thread_current->t_state = THREAD_STATE_RUNNING;

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
    bool not_found = true;

    // TODO: compare current thread pid with pid
    do {
        if (thread->t_pid == pid) {
            not_found = false;
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
    bool not_found = true;

    do {
        if (thread->t_pid == pid) {
            not_found = false;
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
 * 
 * SOLUTION: do not print informations here,
 *  return new list with threads instead
*/

void __list_threads(void) {
    __mutex_lock(&__sched_lock);
    
    struct __thread_control_block *thread = thread_lhead;
    
    if (thread)
        do {
            printk("thread pid=%u, state=%u, flags=%08x, prior=%u\n", thread->t_pid, thread->t_state, thread->t_flags, thread->t_priority);
            printk("  name: %s\n", thread->t_name);
            printk("  esp: %p, cr3: %p\n", thread->t_esp, thread->t_page_dir);
            thread = thread->t_nextt;
        } while (thread != thread_ltail);
    
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