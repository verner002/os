/**
 * @file spinlock.h
 * @author verner002
 * @date 21/02/2026
*/

#pragma once

#include <kernel/atomic.h>

#define SPINLOCK_UNLOCKED   ((atomic_t)0)
#define SPINLOCK_LOCKED     ((atomic_t)1)

typedef struct {
    atomic_t locked;
} spinlock_t;

static inline void spin_loop_hint(void) {
    asm volatile (
        "pause"
        : // no input/output
        : // no input-only
        : // no clobbers
    );
}

void raw_spin_lock(spinlock_t *lock);
void raw_spin_unlock(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
void spin_lock_irq(spinlock_t *lock);
void spin_unlock_irq(spinlock_t *lock);
void spin_lock_irqsave(spinlock_t *lock, unsigned long *flags);
void spin_unlock_irqrestore(spinlock_t *lock, unsigned long *flags);