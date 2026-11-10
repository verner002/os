/**
 * @file io.h
 * @author verner002
 * @date 23/04/2026
*/

#pragma once

#include "kernel/types.h"

#define DUMMY_PORT          0x0080

// "Nd" clobbers tell compiler to use imm8 in/out
// instructions for 8-bit port numbers, otherwise
// dx in/out instructions are used
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile (
        "out %0, %1"
        :
        : "Nd" (port), "a" (value)
        :
    );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;

    asm volatile (
        "in %0, %1"
        : "=a" (value)
        : "Nd" (port)
        :
    );

    return value;
}

static inline void outw(uint16_t port, uint16_t value) {
    asm volatile (
        "out %0, %1"
        :
        : "Nd" (port), "a" (value)
        :
    );
}

static inline uint16_t inw(uint16_t port) {
    uint16_t value;

    asm volatile (
        "in %0, %1"
        : "=a" (value)
        : "Nd" (port)
        :
    );

    return value;
}

static inline void outd(uint16_t port, uint32_t value) {
    asm volatile (
        "out %0, %1"
        :
        : "Nd" (port), "a" (value)
        :
    );
}

static inline uint32_t ind(uint16_t port) {
    uint32_t value;

    asm volatile (
        "in %0, %1"
        : "=a" (value)
        : "Nd" (port)
        :
    );

    return value;
}

// 1-4 ms delay
static inline void io_wait(void) {
    outb(DUMMY_PORT, 0);
}