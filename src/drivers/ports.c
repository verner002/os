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
    asm (
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

    asm (
        "in al, dx"
        : "=a" (v)
        : "d" (r)
        :
    );

    return v;
}

/**
 * __outw
*/

void __outw(uint16_t r, uint16_t v) {
    asm (
        "out dx, al\n\t"
        "mov al, ah\n\t"
        "out dx, al"
        :
        : "d" (r), "a" (v)
        :
    );
}