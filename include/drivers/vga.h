/**
 * Video Graphics Array
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"
#include "bool.h"
#include "drivers/ports.h"

/**
 * Constants
*/

#define VIDEO_MEM 0x000b8000
#define VIDEO_MEM_COLS 80
#define VIDEO_MEM_ROWS 25
#define VIDEO_MEM_DEF_ATTR 7

/**
 * Declarations
*/

void __init_vga(void);
int32_t __putc(uint8_t c);
void __setcurpos(uint32_t line, uint32_t column);
void __scroll_down(void);