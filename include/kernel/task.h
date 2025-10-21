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
#include "hal/vfs.h"
#include "kstdlib/stdio.h"

#define THREAD_RING_0 1

typedef struct __thread_control_block THREAD;

extern THREAD *thread_lcurrent;


__attribute__((interrupt)) void __schedule(void *frame);
int32_t __create_thread(char const *name, int32_t (* main)(int argc, char **argv), uint32_t flags);
int32_t __sched_init(struct __dentry *root_dentry);
int32_t __get_pid(void);
struct __dentry *__get_dentry(void);
void __yield(void);
void __wake_on(bool *alarm);
__attribute__((noreturn)) int32_t __exit(int32_t code);
void __list_threads(void);