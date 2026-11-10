/**
 * @file console.c
 * @author verner002
 * @date 09/09/2026
*/

#include <stddef.h>

//#include <kernel/keyboard.h>
#include <kernel/vga.h>

void con_init(void) {
    vga_init();
    vga_clear();
    vga_set_cursor_pos(0, 0);
}

void con_write(char const *buffer, size_t count) {
    vga_write(buffer);
}