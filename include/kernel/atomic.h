/**
 * @file atomic.h
 * @author verner002
 * @date 10/02/2026
*/

#pragma once

#include "types.h"

typedef int atomic_t;

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
        "xchg %0, %1" // no lock prefix needed
        : "+m" (*ptr), "+r" (value)
        : // no input-only
        : "memory"
    );

    return value;
}