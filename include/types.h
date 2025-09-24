/**
 * Data Types
 * 
 * Author: verner002
*/

#pragma once

/**
 * Types Definitions
*/

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int64_t;
typedef volatile int atomic_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef volatile unsigned int uatomic_t;

#define atomic_inc(x) __asm__ volatile (    \
    "lock inc %0"                           \
    :                                       \
    : "m" (x)                               \
    :                                       \
);

#define atomic_dec(x) __asm__ volatile (    \
    "lock dec %0"                           \
    :                                       \
    : "m" (x)                               \
    :                                       \
);
