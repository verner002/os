/**
 * CPU
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/cpu.h"

/**
 * __enable_interrupts
*/

void __enable_interrupts(void) {
    asm("sti");
}

/**
 * __disable_iunterrupts
*/

void __disable_interrupts(void) {
    asm("cli");
}