/**
 * @file spinlock.c
 * @author verner002
 * @date 21/02/2026
*/

#include <kernel/spinlock.h>
#include <kernel/atomic.h>
#include <kernel/cpu.h>

extern void enable_preemtion(void);
extern void disable_preemtion(void);

void raw_spin_lock(spinlock_t *lock) {
    while (lock->locked || atomic_xchg(&lock->locked, 1))
        spin_loop_hint();
}

void raw_spin_unlock(spinlock_t *lock) {
    memory_barrier();
    lock->locked = 0;
}

// disables preemtion
// irqs remain untouched
// since spinlocks disable scheduler the critical
// sections should be as short as possible
void spin_lock(spinlock_t *lock) {
    disable_preemtion();
    raw_spin_lock(lock);
}

// enables preemtion
// irqs remain untouched
void spin_unlock(spinlock_t *lock) {
    // write to aligned uint32_t is atomic
    raw_spin_unlock(lock);
    enable_preemtion();
}

// disables irqs (thus scheduler)
void spin_lock_irq(spinlock_t *lock) {
    cli();
    raw_spin_lock(lock);
}

// enables irqs (thus scheduler)
void spin_unlock_irq(spinlock_t *lock) {
    raw_spin_unlock(lock);
    sti();
}

// saves irqs state and disables them
// between read_flags and cli only an irq can be
// invoked but since it restores IF flags it doesn't
// matter
void spin_lock_irqsave(spinlock_t *lock, unsigned long *flags) {
    // save IF flag state
    *flags = read_flags();
    cli();
    raw_spin_lock(lock);
}

// restores irqs state
void spin_unlock_irqrestore(spinlock_t *lock, unsigned long *flags) {
    raw_spin_unlock(lock);

    // restore IF flag state
    if (*flags)
        sti();
}