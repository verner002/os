/**
 * @file vga.c
 * @author verner002
 * @date 11/05/2026
*/

#define VGA_ATTR_ADDR_REGISTER          0x03c0
#define VGA_ATTR_WRITE_DATA_REGISTER    0x03c0 // write
#define VGA_ATTR_READ_DATA_REGISTER     0x03c1 // read
#define VGA_MISC_OUTPUT_REGISTER_W      0x03c2
#define VGA_MISC_OUTPUT_REGISTER_R      0x03cc
#define VGA_CRT_CTRL_ADDR_REGISTER      0x03d4
#define VGA_CRT_CTRL_DATA_REGISTER      0x03d5
#define VGA_INPUT_STATUS_1_REGISTER     0x03da

#define VGA_VMEMORY_ATTRIBUTE       7

#include <compiler.h>

#include <kernel/vga.h>
#include <kernel/types.h>
#include <kernel/io.h>

static uint16_t *video_memory = (uint16_t *)VGA_VMEMORY;

static int
    cursor_x = 0,
    cursor_y = 0;

static char color = 0x07;

static uint32_t state = 0;
static uint32_t value = 0;
static uint32_t index = 0;
static uint32_t values[16];

void vga_init(void) {
    // map crt controller to 0x03dx
    outb(VGA_MISC_OUTPUT_REGISTER_W, inb(VGA_MISC_OUTPUT_REGISTER_R) | 0x01);
    
    // reset 0x03c0 flip-flop
    inb(VGA_INPUT_STATUS_1_REGISTER);

    // read address and select attribute mode control register
    uint8_t address = inb(VGA_ATTR_ADDR_REGISTER);
    outb(VGA_ATTR_ADDR_REGISTER, 0x10);

    // read mask and disable blinking
    uint8_t mask = inb(VGA_ATTR_READ_DATA_REGISTER);
    outb(VGA_ATTR_WRITE_DATA_REGISTER, mask & 0xf7);

    // restore address
    outb(VGA_ATTR_ADDR_REGISTER, address);
}

int vga_set_cursor_pos(int x, int y) {
    if (x < 0 || y < 0 || x >= VGA_VMEMORY_WIDTH || y >= VGA_VMEMORY_HEIGHT)
        return VGA_INVALID_CURSOR_POS;

    uint16_t index = y * VGA_VMEMORY_WIDTH + x;

    // cursor position low
    outb(VGA_CRT_CTRL_ADDR_REGISTER, 0x0f);
    outb(VGA_CRT_CTRL_DATA_REGISTER, (uint8_t)index);

    // cursor position high
    outb(VGA_CRT_CTRL_ADDR_REGISTER, 0x0e);
    outb(VGA_CRT_CTRL_DATA_REGISTER, (uint8_t)(index >> 0x08));

    cursor_y = y;
    cursor_x = x;
    return VGA_OK;
}

void vga_clear(void) {
    for (int i = 0; i < VGA_VMEMORY_SIZE; ++i)
        video_memory[i] = ((uint16_t)color << 8) | (uint16_t)' ';

    vga_set_cursor_pos(0, 0);
}

void vga_scroll_down(void) {
    for (int i = 0, j = VGA_VMEMORY_WIDTH; i < VGA_VMEMORY_WIDTH * (VGA_VMEMORY_HEIGHT - 1); ++i, ++j)
        video_memory[i] = video_memory[j]; // j = i + VGA_VMEMORY_WIDTH
    
    for (int i = VGA_VMEMORY_WIDTH * (VGA_VMEMORY_HEIGHT - 1); i < VGA_VMEMORY_SIZE; ++i)
        video_memory[i] = (VGA_VMEMORY_ATTRIBUTE << 8) + ' ';
}

void vga_putc(int x, int y, char c, char a) {
    video_memory[y * VGA_VMEMORY_WIDTH + x] = (uint16_t)(((uint16_t)a << 8) | (uint16_t)c);
}

int vga_printc(char c) {
    if (state == 0) {
        if (c == '\033')
            state = 1;
        else {
            if (c == '\r')
                cursor_x = 0;
            else if (c == '\n') {
                ++cursor_y;
                cursor_x = 0;
            } else if (c == '\b') {
                if (cursor_x || cursor_y) {
                    if (cursor_x)
                        --cursor_x;
                    else if (cursor_y) {
                        cursor_x = VGA_VMEMORY_WIDTH - 1;
                        --cursor_y;
                    }

                    vga_putc(cursor_x, cursor_y, ' ', color);
                }
            } else {
                if (c < 0x20)
                    c = '?'; // invalid character

                vga_putc(cursor_x, cursor_y, c, color);

                if (++cursor_x >= VGA_VMEMORY_WIDTH) {
                    ++cursor_y;
                    cursor_x = 0;
                }
            }

            if (cursor_y >= VGA_VMEMORY_HEIGHT) {
                //--cursor_y;
                cursor_y = VGA_VMEMORY_HEIGHT - 1;
                vga_scroll_down();
            }

            vga_set_cursor_pos(cursor_x, cursor_y);
        }
    } else if (state == 1) {
        if (c == '[')
            state = 2;
        else {
            state = index = 0;
            return -1;
        }
    } else if (state == 2) {
        if (c == ';') {
            // FIXME: check index
            values[index++] = value;
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

                    default:
                        state = value = index = 0;
                        return -1;
                }
            }

            state = value = index = 0;
        } else {
            if (c < '0' || c > '9') {
                state = value = index = 0;
                return -1;
            }

            // FIXME: check value for overflow
            value = 10 * value + c - '0';
        }
    }

    return 0;
}

int vga_write(char const *string) {
    char c;
    int result = 0;

    while (likely((c = *string++) && !result))
        result = vga_printc(c);

    return result;
}