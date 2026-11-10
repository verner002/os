/**
 * @file printk.c
 * @author verner002
 * @date 09/09/2026
*/

#include <kernel/printk.h>
#include <kernel/cpu.h>
#include <kernel/vsprintf.h>
#include <kernel/vga.h>

static char buffer[1024];

int printk(char const *str, ...) {
    int c;

    unsigned long flags;
    save_flags(flags);
    cli();
    
    va_list args;
    va_start(args, str);
    c = vsprintf(buffer, str, args);
    va_end(args);

    vga_write(buffer);
    restore_flags(flags);
    return c;
}