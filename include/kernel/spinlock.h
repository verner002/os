/**
 * @file spinlock.h
 * @author verner002
 * @date 21/02/2026
*/

#pragma once

#include "kernel/atomic.h"

typedef struct {
    atomic_t locked;
} spinlock_t;

static inline void spin_loop_hint(void) {
    asm volatile (
        "pause"
        : // no input/output
        : // no input-only
        : // not clobbers
    );
}

void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);