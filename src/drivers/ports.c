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

void __outb(word r, byte v) {
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

byte __inb(word r) {
    byte v;

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

void __outw(word r, word v) {
    asm (
        "out dx, al\n\t"
        "mov al, ah\n\t"
        "out dx, al"
        :
        : "d" (r), "a" (v)
        :
    );
}