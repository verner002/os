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

static GLOBAL_DESCRIPTOR *gdt; // global descriptor table
static INTERRUPT_DESCRIPTOR *idt; // interrupt descriptor table
static qword counter; // current tick count, 1 tick = 1 ms

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
 * __init_global_descriptor_table
*/

void __init_global_descriptor_table(GLOBAL_DESCRIPTOR *global_descriptor_table) {
    GLOBAL_DESCRIPTOR null_descriptor = {
        .base_high = 0,
        .flag_limit_high = 0,
        .attributes = 0,
        .base_midd = 0,
        .base_low = 0,
        .limit_low = 0
    };

    /**
     * code and data descriptors should be
     * initialized by loader with respect
     * to detected memory
    */

    global_descriptor_table[0] = null_descriptor;
}

/**
 * __set_global_descriptor
*/

void __set_global_descriptor(void) {

}

/**
 * __init_interrupt_descriptor_table
*/

void __init_interrupt_descriptor_table(INTERRUPT_DESCRIPTOR *interrupt_descriptor_table, void (*default_isr)(void)) {
    idt = interrupt_descriptor_table;

    INTERRUPT_DESCRIPTOR __default_descriptor = {
        .offset_low = (word)default_isr,
        .selector = 0x8000,
        .attributes = INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE,
        .offset_high = (word)((word)default_isr >> 16)
    };

    for (unsigned int i = 0; i < 256; ++i) idt[i] = __default_descriptor;
}

/**
 * __set_handler
*/

void __set_handler(byte irq, word selector, byte attributes, void (*isr)(void)) {
    INTERRUPT_DESCRIPTOR id = {
        .offset_low = (word)isr,
        .selector = selector,
        .attributes = attributes,
        .offset_high = (word)((word)isr >> 16)
    };

    idt[irq] = id;
}

/**
 * __init_tick_counter
*/

void __init_tick_counter(void) {
    counter = 0;

    // set pit channel 0 to generate irq
    // map irq to isr __update_tick_counter
}

/**
 * __current_tick_count
*/

qword __current_tick_count(void) {
    return counter;
}