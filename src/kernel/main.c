/**
 * Kernel
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kstdlib/null.h"
//#include "kstdlib/stdio.h"

/**
 * Main
*/

void entry(void) {
    //printf("Kernel ready!\n");

    for (;;) {
        asm("cli");
        asm("hlt");
    }
}