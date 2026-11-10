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
#include "mm/pfa.h"
#include "kernel/ts.h"
#include "mm/heap.h"
#include "kernel/mutex.h"
#include "fs/dentry.h"
#include "kstdlib/file.h"
#include "mm/vmm.h"

#define THREAD_RING_0 1

#define THREAD_PRIORITY_HIGH 0
#define THREAD_PRIORITY_LOW 1

typedef struct task THREAD;

extern THREAD *thread_current;

struct task_fs {
    //uint32_t t_users;
    struct dentry *t_dentry;
};

struct task {
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
    struct task *t_nextt; // next thread
    char const *t_name;
    struct task_fs *t_fs;
    struct vm_area *vm_root; // avl root node
    FILE t_stdin;
    FILE t_stdout;
    FILE t_stderr;
    int argc;
    char **argv;
    int (*main)(int argc, char **argv);
    void (*on_exit)(int exit_code);
};

__attribute__((interrupt)) void schedule(void *frame);
int32_t __create_thread(char const *name, int32_t (* main)(int argc, char **argv), uint32_t flags, uint32_t priority, void (*on_exit_handler)(int exit_code));
int32_t __sched_init(struct dentry *root);
int32_t __get_pid(void);
struct dentry *current_dentry(void);
int32_t __get_state(int32_t pid, uint32_t *state);
int32_t __get_exitcode(int32_t pid, int32_t *exitcode);
struct vm_area **get_vm_root_ptr(void);
void *get_pg_dir(void);
void __yield(void);
void __wake_on(bool *alarm);
__attribute__((noreturn)) int32_t __exit(int32_t code);
void __list_threads(void);
struct task *task_create(char const *name, int32_t (* main)(int argc, char **argv), int argc, char **argv, void (*on_exit_handler)(int exit_code));

int kprocess_create(struct task *task, void *(* routine)(void *));