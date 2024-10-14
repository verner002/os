/**
 * Video Graphics Array
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/vga.h"

/**
 * Static Global Variables
*/

static word *video_memory = (word *)VIDEO_MEM;
static dword
    cursor_x = 0,
    cursor_y = 0;

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
    
    asm (
        "mov dx, 0x03d4\n\t"
        "mov al, 0x0f\n\t"
        "out dx, al\n\t"
        "inc dx\n\t" // mov dx, 0x03d5
        "mov al, cl\n\t"
        "out dx, al\n\t"
        "dec dx\n\t" // mov dx, 0x03d4
        "dec ax\n\t" // mov al, 0x0e
        "out dx, al\n\t"
        "inc dx\n\t" // mov dx, 0x03d5
        "mov al, ch\n\t"
        "out dx, al\n\t"
        :
        :"c" (index)
        :
    );
    
    return 0;
}