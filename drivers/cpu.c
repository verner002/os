/**
 * @file cpu.c
 * @author verner002
 * @date 22/11/2025
*/

#include "drivers/cpu.h"
#include "drivers/isr.h"








static struct global_descriptor gdt[6]; // global descriptor table
static struct interrupt_descriptor idt[256]; // interrupt descriptor table
static struct  tss tss; // task state segment
static uint64_t ticks; // current tick count, 1 tick = 1 ms

/**
 * default_isr
*/

void default_isr(uint8_t vector, struct __interrupt_frame *frame) {
    printk("isr: %u: unhandled interrupt\n", vector);
    for(;;);
}

/**
 * cpuid_features
*/





/**
 * __idt_init
*/

void __idt_init(void) {
    default_isr_init();

    struct idt_ptr idt_ptr = {
        .length = sizeof(idt) - 1,
        .base = (uint32_t)&idt
    };

    asm volatile (
        "lidt %0"
        :
        : "m" (idt_ptr)
        :
    ); // flush idt
}

/**
 * __idt_set_handler
*/

void __idt_set_handler(uint8_t irq, uint16_t selector, uint8_t attributes, void (*isr)(struct __interrupt_frame *frame)) {
    struct interrupt_descriptor id = {
        .offset_low = (uint16_t)((uint32_t)isr & 0xffff),
        .selector = selector,
        .reserved1 = 0,
        .attributes = attributes,
        .offset_high = (uint16_t)(((uint32_t)isr >> 16) & 0xffff)
    };

    idt[irq] = id;
}

/**
 * __init_tick_counter
*/

void __init_tick_counter(void) {
    ticks = 0;
}

/**
 * __update_tick_counter
*/

void __update_tick_counter(void) {
    ++ticks;
}

/**
 * __current_tick_count
*/

uint64_t __current_tick_count(void) {
    return ticks;
}

/**
 * __sleep_ms
 * 
 * Note:
 *  Another task is being processed.
 *  Sleeps the task for a given period of time.
 *  (Remove task from queue and let timer fire an
 *  interrupt after a given time to wake it up?)
*/

/**
 * __delay_ms
 * 
 * Note:
 *  The same task is being processed.
*/

void __delay_ms(uint32_t ms) {
    uint64_t ticks_end = ticks + ms;

    while (ticks < ticks_end);
}