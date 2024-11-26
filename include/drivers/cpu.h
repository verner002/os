/**
 * CPU
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "drivers/8254a.h"

/**
 * Constants
*/

#define INTERRUPT_DESCRIPTOR_PRESENT 0x80
#define INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE 0x0e

/**
 * Types Definitions
*/

typedef struct _global_descriptor GLOBAL_DESCRIPTOR;
typedef struct _interrupt_descriptor INTERRUPT_DESCRIPTOR;

/**
 * Structures
*/

struct _global_descriptor {
    word limit_low;
    word base_low;
    byte base_midd;
    byte attributes;
    byte flag_limit_high;
    byte base_high;
} __attribute__((aligned(1),packed));

struct _interrupt_descriptor {
    word offset_low;
    word selector;
    byte _reserved;
    byte attributes;
    word offset_high;
} __attribute__((aligned(1),packed));

/**
 * Declarations
*/

void __enable_interrupts(void);
void __disable_interrupts(void);
void __init_gdt(GLOBAL_DESCRIPTOR *global_descriptor_table);
void __set_global_descriptor(void);
void __init_idt(INTERRUPT_DESCRIPTOR *interrupt_descriptor_table, void (*default_isr)(void));
void __set_handler(byte irq, word selector, byte attributes, void (*isr)(void));
void __init_tick_counter(void);
void __update_tick_counter(void);
qword __current_tick_count(void);