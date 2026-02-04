/**
 * @file vbe.c
 * @author verner002
 * @date 23/11/2025
*/

#include "drivers/graphics/vbe.h"
#include "mm/vmm.h"
#include "mm/pager.h"

struct __attribute__((__packed__)) __vbe_mode_info {
	uint16_t attributes;		        // deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			        // deprecated
	uint8_t window_b;			        // deprecated
	uint16_t granularity;		        // deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		        // deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			            // number of bytes per horizontal line
	uint16_t width;			            // width in pixels
	uint16_t height;			        // height in pixels
	uint8_t w_char;			            // unused...
	uint8_t y_char;			            // ...
	uint8_t planes;
	uint8_t bpp;			            // bits per pixel
	uint8_t banks;			            // deprecated; total number of banks
	uint8_t memory_model;
	uint8_t bank_size;		            // deprecated; size of a bank, almost always 64 kib but may be 16 kib
	uint8_t image_pages;
	uint8_t reserved0;

	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;

	uint32_t frame_buffer;		        // physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;       // size of memory in the frame_buffer but not being displayed on the screen
    uint8_t reserved1[206];
};

static VBE_MODE_INFO const *mode;
static uint32_t bytes_per_pixel;
static char const *font;
static bool buffering;
static uint32_t buffer;
static uint32_t cursor_x;
static uint32_t cursor_y;

// precalculated values for redraw operation
// (important only when double buffering is
// enabled)
static uint32_t __redraw_move_source;
static uint32_t __redraw_move_destination;
static uint32_t __redraw_move_size;

// precalculated values for scroll down operation
static uint32_t __scroll_move_source;
static uint32_t __scroll_move_destination;
static uint32_t __scroll_move_size;
static uint32_t __scroll_clear_destination;
static uint32_t __scroll_clear_size;
static void (*__scroll_move)(void);

/**
 * __vbe_init
 * 
 * NOTE: only 24-bit (8:8:8) modes and 8x14 fonts supported
*/

int32_t __vbe_init(void const *vbe_info, VBE_MODE_INFO const *vbe_mode_info, char const *font_bitmap, uint32_t bytes_per_char, bool double_buffering) {
    mode = vbe_mode_info;
    font = font_bitmap;

    bytes_per_pixel = mode->bpp / 8;

    uint32_t __buffer = mode->frame_buffer;
    uint32_t __size = bytes_per_pixel * mode->width * mode->height;
    uint32_t __pages = (__size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < __pages; ++i) {
        if (__map_page(__buffer, __buffer, PAGE_READ_WRITE))
            return -1;

        __buffer += PAGE_SIZE;
    }

    buffering = double_buffering;

    if (double_buffering) {
        buffer = __buffer = (uint32_t)pgsalloc(__pages);

        // TODO: clear buffer

        if (!buffer)
            return -1;

        for (uint32_t i = 0; i < __pages; ++i) {
            // RFC: use mmap?
            if (__map_page(__buffer, __buffer, PAGE_READ_WRITE))
                return -1;

            __buffer += PAGE_SIZE;
        }
    } else
        buffer = mode->frame_buffer;

    __buffer = (uint32_t)font_bitmap;
    __pages = (128 * bytes_per_char + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < __pages; ++i) {
        __map_page(__buffer, __buffer, PAGE_READ_WRITE);
        __buffer += PAGE_SIZE;
    }

    cursor_x = cursor_y = 0;
    return 0;
}

void __vbe_redraw(void) {
    uint32_t source = buffer;
    uint32_t destination = mode->frame_buffer;

    if (source == destination)
        return;

    uint32_t size = bytes_per_pixel * mode->width *  mode->height;

    if (!(size & 3)) {
        asm volatile (
            "rep movsd"
            :
            : "c" (size / 4), "D" (destination), "S" (source)
            :
        );
    } else if (!(size & 1)) {
        asm volatile (
            "rep movsw"
            :
            : "c" (size / 2), "D" (destination), "S" (source)
            :
        );
    } else {
        asm volatile (
            "rep movsb"
            :
            : "c" (size), "D" (destination), "S" (source)
            :
        );
    }
}

void __vbe_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= (uint32_t)mode->width || y >= (uint32_t)mode->height)
        return;

    *(uint32_t *)(y * mode->pitch + (x * bytes_per_pixel) + buffer) = color;
}

void __vbe_put_char(uint32_t x, uint32_t y, char character, uint32_t foreground_color, bool transparent, uint32_t background_color) {
    for (uint32_t i = 0; i < 16; ++i)
        for (uint32_t j = 0; j < 8; ++j)
            if (font[character * 16 + i] & (1 << (8 - j - 1)))
                __vbe_put_pixel(x + j, y + i, foreground_color);
            else if (!transparent)
                __vbe_put_pixel(x + j, y + i, background_color);
}

void __vbe_scrolldown(void) {
    uint32_t destination = buffer;
    uint32_t source = buffer + bytes_per_pixel * 16 * mode->width;
    uint32_t size = bytes_per_pixel * mode->width * (mode->height - 16);

    if (!(size & 3)) {
        asm volatile (
            "rep movsd"
            :
            : "c" (size / 4), "D" (destination), "S" (source)
            :
        );
    } else if (!(size & 1)) {
        asm volatile (
            "rep movsw"
            :
            : "c" (size / 2), "D" (destination), "S" (source)
            :
        );
    } else {
        asm volatile (
            "rep movsb"
            :
            : "c" (size), "D" (destination), "S" (source)
            :
        );
    }

    destination = buffer + bytes_per_pixel * mode->width * (mode->height - 16);
    size = bytes_per_pixel * 16 * mode->width;

    if (!(size & 3)) {
        asm volatile (
            "rep stosd"
            :
            : "c" (size / 4), "a" (0), "D" (destination)
            :
        );
    } else if (!(size & 1)) {
        asm volatile (
            "rep stosw"
            :
            : "c" (size / 2), "a" (0), "D" (destination)
            :
        );
    } else {
        asm volatile (
            "rep stosb"
            :
            : "c" (size), "a" (0), "D" (destination)
            :
        );
    }
}