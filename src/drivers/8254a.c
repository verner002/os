/**
 * 8254a
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/8254a.h"

/**
 * __init_pit
*/

void __init_pit(void) {
    // channel 0
    __outb(PIT_MODE_COMMAND_REGISTER, 0x36); // channel 0, low uint8_t / high uint8_t, square wave
    __outb(PIT_CHANNEL_0_DATA_REGISTER, 0x00); // low uint8_t
    __outb(PIT_CHANNEL_0_DATA_REGISTER, 0x00); // high uint8_t
    
    // channel 1
    // we do not use this one

    // channel 2
    __outb(PIT_MODE_COMMAND_REGISTER, 0xb6); // channel 2, low uint8_t / high uint8_t, square wave
    __outb(PIT_CHANNEL_2_DATA_REGISTER, 0x00); // low uint8_t
    __outb(PIT_CHANNEL_2_DATA_REGISTER, 0x00); // high uint8_t
}