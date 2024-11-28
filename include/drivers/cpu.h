/**
 * CPU
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "drivers/8259a.h"

/**
 * Constants
*/

#define INTERRUPT_DESCRIPTOR_PRESENT 0x80
#define INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE 0x0e

/**
 * Types Definitions
*/

typedef struct _global_descriptor GLOBAL_DESCRIPTOR;
typedef struct _idt_ptr IDT_PTR;
typedef struct _interrupt_descriptor INTERRUPT_DESCRIPTOR;
typedef struct _interrupt_frame INTERRUPT_FRAME;

/**
 * Structures
*/

struct __attribute__((__packed__)) _global_descriptor {
    word limit_low;
    word base_low;
    byte base_midd;
    byte attributes;
    byte flag_limit_high;
    byte base_high;
};

struct __attribute__((__packed__)) _idt_ptr {
    word length;
    dword base;
};

struct __attribute__((__packed__)) _interrupt_descriptor {
    word offset_low;
    word selector;
    byte _reserved;
    byte attributes;
    word offset_high;
};

struct __attribute__((__packed__)) _interrupt_frame {
    // TODO: implement
};

/**
 * Declarations
*/

void __enable_interrupts(void);
void __disable_interrupts(void);
void __init_gdt(GLOBAL_DESCRIPTOR *global_descriptor_table);
void __set_global_descriptor(void);
void __init_idt(INTERRUPT_DESCRIPTOR *interrupt_descriptor_table, void (*default_isr)(INTERRUPT_FRAME *frame));
void __set_handler(byte irq, word selector, byte attributes, void (*isr)(INTERRUPT_FRAME *frame));
void __init_tick_counter(void);
__attribute__((interrupt)) void __update_tick_counter(INTERRUPT_FRAME *frame);
unsigned long __current_tick_count(void);
void __delay_ms(unsigned int ms);