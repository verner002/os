/**
 * @file graphix.h
 * @author verner002
 * @date 22/11/2025
*/

#pragma once

#include "types.h"

// TODO: add mode? (text/graphics)
struct __display {
    uint32_t cur_x;     // cursor x
    uint32_t cur_y;     // cursor y
    uint32_t width;     // width in pixels
    uint32_t height;    // height in pixels
    uint32_t pitch;     // bytes per line
    uint32_t bpp;       // bits per pixel
    void *frame_buffer; // frame buffer
};

struct __font {
    uint32_t width;
    uint32_t height;
    char *bitmap;
};

int32_t __graphix_init(void);
int32_t __graphix_putc(uint8_t c);