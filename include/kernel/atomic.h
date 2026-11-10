/**
 * @file atomic.h
 * @author verner002
 * @date 10/02/2026
*/

#pragma once

typedef int atomic_t;

static inline void memory_barrier(void) {
    asm volatile ("" ::: "memory");
}

#ifdef CONFIG_I586
static inline void memory_fence(void) {
    // must be at least compiler memory barrier
    asm volatile ("mfence" ::: "memory");
}

static inline void memory_load_fence(void) {
    asm volatile ("lfence" ::: "memory");
}

static inline void memory_store_fence(void) {
    asm volatile ("sfence" ::: "memory");
}
#endif

static inline void atomic_increment(atomic_t *ptr) {
    asm volatile (
        "lock inc %0"
        : "+m" (*ptr)
        : // no input-only
        : "memory", "cc"
    );
}

static inline void atomic_decrement(atomic_t *ptr) {
    asm volatile (
        "lock dec %0"
        : "+m" (*ptr)
        : // no input-only
        : "memory", "cc"
    );
}

static inline int atomic_fetch_add(atomic_t *ptr, int value) {
    asm volatile (
        "lock xadd %0, %1"
        : "+m" (*ptr), "+r" (value)
        : // no input-only
        : "memory", "cc"
    );

    return value;
}

static inline int atomic_xchg(atomic_t *ptr, int value) {
    asm volatile (
        "lock xchg %0, %1" // no lock prefix needed
        : "+m" (*ptr), "+r" (value)
        : // no input-only
        : "memory"
    );

    return value;
}