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