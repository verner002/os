/**
 * Video Graphics Array
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "types.h"

/**
 * Constants
*/

#define VIDEO_MEM 0x000b8000
#define VIDEO_MEM_COLS 80
#define VIDEO_MEM_DEF_ATTR 7

/**
 * Declarations
*/

int __putc(byte c, byte a);
int __setcurpos(dword l, dword c);