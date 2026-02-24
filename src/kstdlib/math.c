/**
 * @file math.c
 * @author verner002
 * @date 10/10/2025
*/

#include "kstdlib/math.h"

/**
 * max
*/

int32_t max(int32_t n, int32_t m) {
    return n >= m ? n : m;
}

/**
 * log2b
*/

uint32_t log2b(uint32_t n) {
    return
        (((n & 0xaaaaaaaa) != 0) << 0) |
        (((n & 0xcccccccc) != 0) << 1) |
        (((n & 0xf0f0f0f0) != 0) << 2) |
        (((n & 0xff00ff00) != 0) << 3) |
        (((n & 0xffff0000) != 0) << 4);
}

/**
 * log10
 * 
 * FIXME: with -O0 this function returns weird results
*/

uint32_t log10(uint32_t n) {
    static uint8_t digits[] = { 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10 };
    static uint32_t powers[] = { 0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
    static uint32_t log;

    asm volatile (
        "push eax\n\t"
        "bsr eax, ebx\n\t"
        "movzx eax, byte ptr [ecx+eax+1]\n\t"
        "cmp ebx, dword ptr [edx+eax*4]\n\t"
        "sbb al, 0\n\t"
        "mov %0, eax\n\t"
        "pop eax"
        : "=m" (log)
        : "b" (n), "c" (digits), "d" (powers)
        :
    );

    return log;
}

int digits(int n) {
    if (!n)
        return 1;
    
    return log10(n) + 1;
}

/**
 * __udivdi3
 * 
 * TODO: i'll try to optimize these division functions in future
*/

uint64_t __udivdi3(uint64_t dividend, uint64_t divisor) {
    if (!divisor)
        return 0;

    uint64_t quotient = 0;
    uint64_t remainder = 0;

    for (int32_t i = 63; i >= 0; --i) {
        remainder <<= 1;
        remainder |= (dividend >> i) & 1;

        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= (1ULL << i);
        }
    }

    return quotient;
}

/**
 * __umoddi3
*/

uint64_t __umoddi3(uint64_t dividend, uint64_t divisor) {
    if (!divisor)
        return 0;

    uint64_t remainder = 0;

    for (int32_t i = 63; i >= 0; --i) {
        remainder <<= 1;
        remainder |= (dividend >> i) & 1;

        if (remainder >= divisor)
            remainder -= divisor;
    }

    return remainder;
}

void __udivmoddi4(uint64_t dividend, uint64_t divisor, uint64_t *q, uint64_t *r) {
    if (!divisor)
        return;

    uint64_t quotient = 0;
    uint64_t remainder = 0;

    for (int32_t i = 63; i >= 0; --i) {
        remainder <<= 1;
        remainder |= (dividend >> i) & 1;

        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= (1ULL << i);
        }
    }

    *q = quotient;
    *r = remainder;
}