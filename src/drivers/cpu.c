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
static unsigned long ticks; // current tick count, 1 tick = 1 ms

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
 * __init_gdt
*/

void __init_gdt(GLOBAL_DESCRIPTOR *global_descriptor_table) {
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
 * __init_idt
*/

void __init_idt(INTERRUPT_DESCRIPTOR *interrupt_descriptor_table, void (*default_isr)(INTERRUPT_FRAME *frame)) {
    idt = interrupt_descriptor_table;

    IDT_PTR idt_ptr = {
        .length = 0x07ff,
        .base = (dword)idt
    };

    INTERRUPT_DESCRIPTOR __default_descriptor = {
        .offset_low = (word)((dword)default_isr & 0xffff),
        .selector = 0x0008, // code segment selector
        .attributes = INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE,
        .offset_high = (word)(((dword)default_isr >> 16) & 0xffff)
    };

    for (unsigned int i = 0; i < 256; ++i) idt[i] = __default_descriptor;

    asm (
        "lidt %0"
        :
        : "m" (idt_ptr)
        :
    );
}

/**
 * __set_handler
*/

void __set_handler(byte irq, word selector, byte attributes, void (*isr)(INTERRUPT_FRAME *frame)) {
    INTERRUPT_DESCRIPTOR id = {
        .offset_low = (word)((dword)isr & 0xffff),
        .selector = selector,
        .attributes = attributes,
        .offset_high = (word)(((dword)isr >> 16) & 0xffff)
    };

    idt[irq] = id;
}

/**
 * __init_tick_counter
*/

void __init_tick_counter(void) {
    ticks = 0;

    // set pit channel 0 to generate irq
    // map irq to isr __update_tick_counter
}

__attribute__((interrupt)) void __update_tick_counter(INTERRUPT_FRAME *frame) {
    __send_eoi(0x00);
    ++ticks;
}

/**
 * __current_tick_count
*/

unsigned long __current_tick_count(void) {
    return ticks;
}

/**
 * __sleep_ms
 * 
 * Note:
 *  Another task is begin processed.
 *  Sleeps the task for a given period of time.
 *  (Remove task from queue and let timer fire an
 *  interrupt after a given time to return it?)
*/

/**
 * __delay_ms
 * 
 * Note:
 *  The same task is begin processed.
*/

void __delay_ms(unsigned int ms) {
    unsigned long ticks_end = ticks + ms;

    while (ticks < ticks_end);
}