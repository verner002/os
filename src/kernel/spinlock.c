/**
 * @file spinlock.c
 * @author verner002
 * @date 21/02/2026
*/

#include "kernel/spinlock.h"
#include "kernel/atomic.h"

void spin_lock(spinlock_t *lock) {
    while (lock->locked || atomic_xchg(&lock->locked, 1))
        spin_loop_hint();
}

void spin_unlock(spinlock_t *lock) {
    // write to aligned uint32_t is atomic
    lock->locked = 0;
}