/**
 * Video Graphics Array
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/vga.h"

/**
 * Static Global Variables
*/

static word *video_memory = (word *)VIDEO_MEM;
static dword
    cursor_x = 0,
    cursor_y = 0;

/**
 * __init_vga
*/

void __init_vga(void) {
    __outb(VGA_MISCELLANEOUS_OUTPUT_REGISTER_W, __inb(VGA_MISCELLANEOUS_OUTPUT_REGISTER_R) | 0x01); // map crt controller to 0x03dx
}

/**
 * __putc
*/

int __putc(byte c, byte a) {
    word data = ((word)(a + !a * VIDEO_MEM_DEF_ATTR) << 0x08) | c;
        // (a ? a : VIDEO_MEM_DEF_ATTR)

    video_memory[cursor_y * VIDEO_MEM_COLS + cursor_x] = data;

    return 0; // error code
}

/**
 * __setcurpos
*/

int __setcurpos(dword l, dword c) {
    cursor_y = l;
    cursor_x = c;

    word index = cursor_y * VIDEO_MEM_COLS + cursor_x;

    __outb(VGA_CRT_CONTROLLER_ADDRESS_REGISTER, 0x0f); // cursor location low
    __outb(VGA_CRT_CONTROLLER_DATA_REGISTER, (byte)index);
    __outb(VGA_CRT_CONTROLLER_ADDRESS_REGISTER, 0x0e); // cursor location high
    __outb(VGA_CRT_CONTROLLER_DATA_REGISTER, (byte)(index >> 0x08)); // compiler should optimize this to use low and high part of an register
    
    return 0;
}