/**
 * Speaker
 * 
 * Author: verner002
 * 
 * TODO:
 *  1) use functions from 8254a.h to interact with pit
*/

/**
 * Includes
*/

#include "drivers/speaker.h"

/**
 * __init_speaker
*/

void __init_speaker(void) {
    __disable_speaker();
    __outb(PIT_MODE_COMMAND_REGISTER, 0xb6); // channel 2, low byte / high byte, square wave
}

/**
 * __enable_speaker
*/

void __enable_speaker(void) {
    __outb(PPI_PORT_B, __inb(PPI_PORT_B) | 0x03);
}

/**
 * __enable_speaker
*/

void __disable_speaker(void) {
    __outb(PPI_PORT_B, __inb(PPI_PORT_B) & 0xfc);
}

/**
 * __play_note
*/

void __play_note(word f) {
    __outw(PIT_CHANNEL_2_DATA_REGISTER, 0x001234de / f);
    __enable_speaker();
}