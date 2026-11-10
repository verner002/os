/**
 * @file panic.c
 * @author verner002
 * @date 11/05/2026
*/

#include <kernel/panic.h>
#include <kernel/printk.h>

void panic(void) {
    printk("KERNEL PANIC");

    for (;;) {
        asm volatile (
            "cli\n\t"
            "hlt"
            :
            :
            :
        );
    }
}