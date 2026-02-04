/**
 * @file graphix.c
 * @author verner002
 * @date 22/11/2025
*/

#include "bool.h"
#include "macros.h"
#include "kernel/mutex.h"
#include "drivers/graphics/graphix.h"

// vga and vbe don't support multiple displays
// so let's keep it simple
static struct __display display = {
    .cur_x = 0,
    .cur_y = 0,
    .width = 1024,
    .height = 768
};

static struct {
    bool initialized;
    int32_t (* init)(void const *vbe_info, VBE_MODE_INFO const *vbe_mode_info, char const *font_bitmap, uint32_t bytes_per_char, bool double_buffering);
    void (* put_char)(uint32_t x, uint32_t y, char c, uint32_t fcolor, bool transparent, uint32_t bcolor);
    void (* scrolldown)(void);
} graphix_driver = {
    .initialized = false,
    .init = &__vbe_init,
    .put_char = &__vbe_put_char,
    .scrolldown = &__vbe_scrolldown
};

/**
 * __graphix_init
*/

int32_t __graphix_init(void const *vbe_info, VBE_MODE_INFO const *vbe_mode_info, char const *font_bitmap, uint32_t bytes_per_char) {
    // implicitly enable double buffering
    int32_t error = graphix_driver.init(vbe_info, vbe_mode_info, font_bitmap, bytes_per_char, false);
    graphix_driver.initialized = true;
    return error;
}

/**
 * __graphix_putc
*/

int32_t __graphix_putc(uint8_t c) { 
    static bool mutex = false;
    static uint32_t state = 0;
    static uint32_t value = 0;
    static uint32_t index = 0;
    static uint32_t values[16];

    if (unlikely(!graphix_driver.initialized))
        return -1;

    __mutex_lock(&mutex);

    switch (state) {
        case 0:
            if (unlikely(c == '\033')) {
                state = 1;
                break;
            }

            switch (c) {
                case '\r':
                    display.cur_x = 0;
                    break;

                case '\n':
                    display.cur_x = 0;
                    ++display.cur_y;
                    break;

                case '\b':
                    // cursor position (0,0)
                    if (!display.cur_x && !display.cur_y)
                        break;

                    if (display.cur_x)
                        --display.cur_x;
                    else/* if (display.cur_y)*/ {
                        display.cur_x = (display.width / 8) - 1;
                        --display.cur_y;
                    }

                    graphix_driver.put_char(display.cur_x * 8, display.cur_y * 16, ' ', RGB(255, 255, 255), false, RGB(0, 0, 0));
                    break;

                case '\t':
                    // tab indent is 5
                    if (display.cur_x + 5 < (display.width / 8))
                        display.cur_x = (display.cur_x + 5) - ((display.cur_x + 5) % 5);
                    break;

                default:
                    if (c < 0x20 || c > 0x7f)
                        c = '?'; // unknown character

                    graphix_driver.put_char(display.cur_x * 8, display.cur_y * 16, c, RGB(255, 255, 255), false, RGB(0, 0, 0));

                    if (++display.cur_x >= (display.width / 8)) {
                        display.cur_x = 0;
                        ++display.cur_y;
                    }
                    break;
            }

            if (display.cur_y >= (display.height / 16)) {
                display.cur_y = (display.height / 16) - 1;
                graphix_driver.scrolldown();
            }

            // TODO: set hardware/software cursor position
            break;

        case 1:
            if (c == '[') {
                state = 2;
                break;
            }

            state = index = 0;
            __mutex_unlock(&mutex);
            return -1;

        case 2:
            switch (c) {
                case ';':
                    values[index++] = value;
                    value = 0;
                    break;
                
                case 'm':
                    values[index++] = value;

                    state = value = index = 0;
                    break;

                default:
                    if (c < '0' || c > '9') {
                        state = value = index = 0;
                        __mutex_unlock(&mutex);
                        return -1;
                    }

                    // RFC: check for overflow?
                    value = 10 * value + c - '0';
                    break;
            }
            break;
    }

    __mutex_unlock(&mutex);
    return 0;
}