/**
 * @file cpu.h
 * @author verner002
 * @date 19/07/2026
*/

#pragma once

// "memory" clobbers prevent compiler from moving
// cli/sti instruction
static inline void cli(void) {
    asm volatile (
        "cli"
        :
        :
        : "memory"
    );
}

static inline void sti(void) {
    asm volatile (
        "sti"
        :
        :
        : "memory"
    );
}

#define save_flags(x) asm volatile (    \
    "pushf\n\t"                         \
    "pop %0"                            \
    : "=g" (x)                          \
    :                                   \
    : "memory"                          \
);

#define restore_flags(x) asm volatile ( \
    "push %0\n\t"                       \
    "popf"                              \
    :                                   \
    : "g" (x)                           \
    : "memory"                          \
);
