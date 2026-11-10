/**
 * @file sched.h
 * @author verner002
 * @date 08/04/2026
*/

#include <types.h>
#include "mm/vmm.h"

typedef uint64_t ticks_t;

// process/thread (depends on context)
struct task {
    // architecture dependend fields
    // these are hardcoded, do not change
    // unless you know what you are doing
    struct cr3 cr3;             // task cr3
    uint32_t esp;               // task esp
    uint32_t stack;             // task stack
    uint32_t kesp;              // kernel esp
    uint32_t kstack;            // kernel stack
    
    // identification
    int parent_pid;             // parent pid
    int pid;                    // task pid

    // runtime
    int priority;               // task priority
    ticks_t mticks;             // task max ticks before dispatch
    ticks_t ticks;              // task current ticks

    // environment
    struct task_fs *fs_info;    // task fs info
    struct vm_area *vm_space;   // vmas avl tree node

    // execution
    void *data;                 // task data
    void *(*routine)(void *);   // task routine

    // signals
    void (*before_start)(void *);
    void (*after_start)(void *);
    void (*before_exit)(void *);
    void (*after_exit)(void *);
};

extern volatile uint64_t ticks;
extern volatile struct task_list *running;

void ticks_update(void);
uint64_t ticks_now(void);