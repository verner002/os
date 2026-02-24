/**
 * @file panic.c
 * @author verner002
 * @date 08/02/2026
*/

#include "kernel/panic.h"
#include "kstdlib/stdio.h"

void panic(void) {
    printk("\033[31mKERNEL PANIC\n");

    for (;;) {
        asm volatile (
            "cli\n\t"
            "hlt"
        );
    }
}