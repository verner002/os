/**
 * Kernel
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kstd/null.h"
#include "kstd/stdio.h"

/**
 * Main
*/

void entry(void) {
    printf("Kernel ready!\n");

    for (;;) {
        asm("cli");
        asm("hlt");
    }
}