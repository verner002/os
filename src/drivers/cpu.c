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
 * Static Global Variable
*/

static unsigned long current_tick_count; // 1 tick = 1 ms

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

/**
 * __init_tick_counter
*/

void __init_tick_counter(void) {
    current_tick_count = 0;

    // set pit channel 0 to generate irq
    // map irq to isr __update_tick_counter
}

/**
 * __current_tick_count
*/

unsigned long __current_tick_count(void) {
    return current_tick_count;
}