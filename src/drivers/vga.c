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

static uint16_t *video_memory = (uint16_t *)VIDEO_MEM;
static uint32_t
    cursor_x = 0,
    cursor_y = 0;
    static uint8_t color = 0x07;
static uint32_t state = 0;
static uint32_t value = 0;
static uint32_t index = 0;
static uint32_t values[16]; // TODO: define constant

/**
 * __init_vga
*/

void __init_vga(void) {
    __outb(VGA_MISCELLANEOUS_OUTPUT_REGISTER_W, __inb(VGA_MISCELLANEOUS_OUTPUT_REGISTER_R) | 0x01); // map crt controller to 0x03dx
    
    __inb(VGA_INPUT_STATUS_1_REGISTER); // reset 0x03c0 flip-flop
    uint8_t address = __inb(VGA_ATTRIBUTE_ADDRESS_REGISTER); // read address
    __outb(VGA_ATTRIBUTE_ADDRESS_REGISTER, 0x10); // select attribute mode control register
    uint8_t mask = __inb(VGA_ATTRIBUTE_READ_DATA_REGISTER); // read mask
    __outb(VGA_ATTRIBUTE_WRITE_DATA_REGISTER, mask & 0xf7); // disable blinking
    __outb(VGA_ATTRIBUTE_ADDRESS_REGISTER, address); // restore address
}

/**
 * __putc
*/

int32_t __putc(uint8_t c) {
    if (state == 0) {
        if (c == '\033') state = 1;
        else {
            if (c == '\r') cursor_x = 0;
            else if (c == '\n') {
                ++cursor_y;
                cursor_x = 0;
            } else {
                if (c < 0x20 || c > 0xfe) c = '?'; // invalid character

                video_memory[cursor_y * VIDEO_MEM_COLS + cursor_x] = ((uint16_t)color << 8) | (uint16_t)c;

                if (++cursor_x >= VIDEO_MEM_COLS) {
                    ++cursor_y;
                    cursor_x = 0;
                }
            }

            if (cursor_y >= VIDEO_MEM_ROWS) {
                cursor_y = VIDEO_MEM_ROWS - 1; // this is safer than --cursor_y
                __scroll_down();
            }

            __setcurpos(cursor_y, cursor_x);
        }
    } else if (state == 1) {
        if (c == '[') state = 2;
        else {
            state = index = 0;
            return -1;
        }
    } else if (state == 2) {
        if (c == ';') {
            values[index++] = value; // FIXME: check index
            value = 0;
        } else if (c == 'm') {
            values[index++] = value;

            for (uint32_t i = 0; i < index; ++i) {
                uint8_t fgc = color & 0x0f;
                uint8_t bgc = color & 0xf0;

                switch (values[i]) {
                    case 30: color = bgc | 0x00; break;
                    case 31: color = bgc | 0x04; break;
                    case 32: color = bgc | 0x02; break;
                    case 33: color = bgc | 0x06; break;
                    case 34: color = bgc | 0x01; break;
                    case 35: color = bgc | 0x05; break;
                    case 36: color = bgc | 0x03; break;
                    case 37: color = bgc | 0x07; break;

                    case 40: color = 0x00 | fgc; break;
                    case 41: color = 0x40 | fgc; break;
                    case 42: color = 0x20 | fgc; break;
                    case 43: color = 0x60 | fgc; break;
                    case 44: color = 0x10 | fgc; break;
                    case 45: color = 0x50 | fgc; break;
                    case 46: color = 0x30 | fgc; break;
                    case 47: color = 0x70 | fgc; break;

                    case 90: color = bgc | 0x08; break;
                    case 91: color = bgc | 0x0c; break;
                    case 92: color = bgc | 0x0a; break;
                    case 93: color = bgc | 0x0e; break;
                    case 94: color = bgc | 0x09; break;
                    case 95: color = bgc | 0x0d; break;
                    case 96: color = bgc | 0x0b; break;
                    case 97: color = bgc | 0x0f; break;

                    case 100: color = 0x80 | fgc; break;
                    case 101: color = 0xc0 | fgc; break;
                    case 102: color = 0xa0 | fgc; break;
                    case 103: color = 0xe0 | fgc; break;
                    case 104: color = 0x90 | fgc; break;
                    case 105: color = 0xd0 | fgc; break;
                    case 106: color = 0xb0 | fgc; break;
                    case 107: color = 0xf0 | fgc; break;
                    default: state = value = index = 0; return -1;
                }
            }

            state = value = index = 0;
        } else {
            if (c < '0' || c > '9') {
                state = value = index = 0;
                return -1;
            }

            value = 10 * value + c - '0'; // FIXME: check value
        }
    }

    return 0;
}

/**
 * __setcurpos
*/

void __setcurpos(uint32_t line, uint32_t column) {
    cursor_y = line;
    cursor_x = column;

    uint16_t index = cursor_y * VIDEO_MEM_COLS + cursor_x;

    __outb(VGA_CRT_CONTROLLER_ADDRESS_REGISTER, 0x0f); // cursor location low
    __outb(VGA_CRT_CONTROLLER_DATA_REGISTER, (uint8_t)index);
    __outb(VGA_CRT_CONTROLLER_ADDRESS_REGISTER, 0x0e); // cursor location high
    __outb(VGA_CRT_CONTROLLER_DATA_REGISTER, (uint8_t)(index >> 0x08)); // compiler should optimize this to use low and high part of an register
}

/**
 * __scroll_down
*/

void __scroll_down(void) {
    for (uint32_t i = 0, j = VIDEO_MEM_COLS; i < VIDEO_MEM_COLS * (VIDEO_MEM_ROWS - 1); ++i, ++j) video_memory[i] = video_memory[j]; // j = i + VIDEO_MEM_COLS
    for (uint32_t i = VIDEO_MEM_COLS * (VIDEO_MEM_ROWS - 1); i < VIDEO_MEM_COLS * VIDEO_MEM_ROWS; ++i) video_memory[i] = (VIDEO_MEM_DEF_ATTR << 8) + ' ';
}