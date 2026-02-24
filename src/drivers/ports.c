/**
 * Ports
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/ports.h"

/**
 * __outb
*/

void __outb(uint16_t r, uint8_t v) {
    asm volatile (
        "out dx, al"
        :
        : "d" (r), "a" (v)
        :
    );
}

/**
 * __inb
*/

uint8_t __inb(uint16_t r) {
    uint8_t v;

    asm volatile (
        "in al, dx"
        : "=a" (v)
        : "d" (r)
        :
    );

    return v;
}

/**
 * __inw
*/

uint16_t __inw(uint16_t r) {
    uint16_t v;

    asm volatile (
        "in %0, %1"
        : "=a" (v)
        : "Nd" (r)
        :
    );

    return v;
}

/**
 * __outw
 * 
 * RFC: couldn't we just use `out dx, ax'?
*/

void __outw(uint16_t r, uint16_t v) {
    asm volatile (
        "out dx, ax"
        :
        : "d" (r), "a" (v)
        :
    );
}

/**
 * __ind
*/

uint32_t __ind(uint16_t r) {
    uint32_t v;

    asm volatile (
        "in eax, dx"
        : "=a" (v)
        : "d" (r)
        :
    );

    return v;
}

/**
 * __outd
*/

void __outd(uint16_t r, uint32_t v) {
    asm volatile (
        "out dx, eax"
        :
        : "d" (r), "a" (v)
        :
    );
}