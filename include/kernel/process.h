/**
 * @file process.h
 * @author verner002
 * @date 22/08/2026
*/

#pragma once

#include <stddef.h>

#include <kernel/types.h>
#include <kernel/list.h>
#include <kernel/atomic.h>
#include <kernel/dcache.h>

typedef int pid_t;

// maximum number of process
#define NPROC               32

#define NFDESC              16

#define PROC_FREE           0
#define PROC_RUNNING        1
#define PROC_READY          2
#define PROC_SUSPENDED      3
#define PROC_WAITING        4

#define STACK_CANARY    0xcafecafe

#define ISBADPID(x) ((x) < 0 || (x) >= NPROC || proctab[(x)].state == PROC_FREE)

// early declarations
struct file;

struct process {
    pid_t pid;
    unsigned char state;
    unsigned char priority;
    unsigned long s_time;
    unsigned long u_time;
    struct dentry *cwd;
    uint32_t cr3;
    unsigned long stack_base;
    unsigned long stack_ptr;
    size_t stack_size;
    unsigned long kstack_base;
    unsigned long kstack_ptr;
    size_t kstack_size;
    pid_t parent_pid;
    struct list wait_list; // semaphore wait list
    struct file *fdescs[NFDESC]; // device numbers
};

struct semaphore {
    //spinlock_t lock; -- we're ok with disabling irqs for now
    atomic_t count; // semaphore value
    struct list *wait_list; // wait list head
};

struct regs {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t user_esp;
    uint32_t ss;
};

// number of active processes
extern volatile size_t nproc;
// currently running process
extern volatile pid_t cproc;
// process table
extern struct process proctab[];

pid_t sys_fork(struct regs *regs);
pid_t sys_create(void (*function)(void), unsigned char priority, size_t stack_size);
pid_t getpid(void);
void delay_ms(uint32_t ms);
int suspend(pid_t pid);
int resume(pid_t pid);
int wait(struct semaphore *sem);
int signal(struct semaphore *sem);
int kill(pid_t pid);
//void enable_preemtion(void);
//void disable_preemtion(void);
void schedule(void);