/**
 * Kernel
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "null.h"
#include "kstdlib/stdio.h"

/**
 * Main
*/

void entry() {
    printf("Kernel ready!\n");

    for (;;) {
        asm("cli");
        asm("hlt");
    }
}