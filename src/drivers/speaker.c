/**
 * Speaker
 * 
 * Author: verner002
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

    asm (
        "mov al, 0xb6\n\t"
        "out 0x43, al" // channel 2, lowbyte / highbyte, square wave
    );
}

/**
 * __enable_speaker
*/

void __enable_speaker(void) {
    asm (
        "in al, 0x61\n\t"
        "or al, 0x03\n\t"
        "out 0x61, al"
    );
}

/**
 * __enable_speaker
*/

void __disable_speaker(void) {
    asm (
        "in al, 0x61\n\t"
        "and al, 0xfc\n\t"
        "out 0x61, al"
    );
}

/**
 * __play_note
*/

void __play_note(word f) {
    word value = 0x001234de / f;
    
    asm(
        "out 0x42, al\n\t"
        "mov al, ah\n\t"
        "out 0x42, al"
        :
        : "a" (value)
        :
    );

    __enable_speaker();
}