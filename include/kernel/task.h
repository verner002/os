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
#include "mm/pager.h"
#include "kernel/ts.h"
#include "mm/heap.h"
#include "kernel/mutex.h"
#include "fs/dentry.h"
#include "kstdlib/file.h"

#define THREAD_RING_0 1

#define THREAD_PRIORITY_HIGH 0
#define THREAD_PRIORITY_LOW 1

typedef struct __thread_control_block THREAD;

extern THREAD *thread_current;

__attribute__((interrupt)) void __schedule(void *frame);
int32_t __create_thread(char const *name, int32_t (* main)(int argc, char **argv), uint32_t flags, uint32_t priority, void (*on_exit_handler)(int exit_code));
int32_t __sched_init(struct dentry *root);
int32_t __get_pid(void);
struct dentry *current_dentry(void);
int32_t __get_state(int32_t pid, uint32_t *state);
int32_t __get_exitcode(int32_t pid, int32_t *exitcode);
void __yield(void);
void __wake_on(bool *alarm);
__attribute__((noreturn)) int32_t __exit(int32_t code);
void __list_threads(void);
int create_thread(char const *name, int32_t (* main)(int argc, char **argv), int argc, char **argv, void (*on_exit_handler)(int exit_code));