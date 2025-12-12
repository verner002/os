/**
 * @file vbe.h
 * @author verner002
 * @date 23/11/2025
*/

#pragma once

#include "types.h"
#include "bool.h"

typedef struct __attribute__((__packed__)) __vbe_mode_info VBE_MODE_INFO;

int32_t __vbe_init(void const *vbe_info, VBE_MODE_INFO const *vbe_mode_info, char const *font_bitmap, uint32_t bytes_per_char, bool double_buffering);
void __vbe_redraw(void);
void __vbe_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void __vbe_put_char(uint32_t x, uint32_t y, char character, uint32_t foreground_color, bool transparent, uint32_t background_color);
void __vbe_scrolldown(void);