/**
 * @file vga.h
 * @author verner002
 * @date 11/05/2026
*/

#pragma once

#include <kernel/system.h>

#define VGA_VMEMORY             ((void *)(0x000b8000 + KERNEL_VIRT_BASE))
#define VGA_VMEMORY_WIDTH       (80)
#define VGA_VMEMORY_HEIGHT      (25)
#define VGA_VMEMORY_SIZE        (VGA_VMEMORY_WIDTH*VGA_VMEMORY_HEIGHT)

#define VGA_OK                  0
#define VGA_INVALID_CURSOR_POS  1

void vga_init(void);
int vga_set_cursor_pos(int x, int y);
void vga_clear(void);
void vga_putc(int x, int y, char c, char a);
int vga_printc(char c);
int vga_write(char const *string);