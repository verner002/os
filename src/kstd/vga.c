/**
 * Video Graphics Array
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kstd/vga.h"

/**
 * Static Global Variables
*/

word *video_memory = (word *)VIDEO_MEM;
dword
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
    return 0;
}