/**
 * @file graphix.c
 * @author verner002
 * @date 22/11/2025
*/

#include "drivers/graphics/graphix.h"

// vga and vbe don't support multiple displays
// so let's keep it simple
static struct __display display;

/**
 * __graphix_init
*/

int32_t __graphix_init(void) {
    return -1;
}

/**
 * __graphix_putc
*/

int32_t __graphix_putc(uint8_t c) {
    return -1;
}