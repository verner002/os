/**
 * Kernel
 * 
 * Author: verner002
*/

/**
 * Main
*/

void entry(void) {
    for (;;) {
        asm("cli");
        asm("hlt");
    }
}