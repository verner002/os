/**
 * CPU
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "kernel/task.h"
#include "kernel/userland.h"
#include "drivers/8259a.h"
#include "kstdlib/string.h"
#include "kstdlib/stdio.h"

/**
 * Constants
*/

#define INTERRUPT_DESCRIPTOR_PRESENT 0x80
#define INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE 0x0e

/**
 * Types Definitions
*/

typedef struct _gdt_ptr GDT_PTR;
typedef struct _global_descriptor GLOBAL_DESCRIPTOR;
typedef struct _idt_ptr IDT_PTR;
typedef struct _interrupt_descriptor INTERRUPT_DESCRIPTOR;
typedef struct _task_state_segment TASK_STATE_SEGMENT;
typedef struct _interrupt_frame INTERRUPT_FRAME;

/**
 * Structures
*/

struct __attribute__((__packed__)) _gdt_ptr {
    uint16_t length;
    uint32_t base;
};

struct __attribute__((__packed__)) _global_descriptor {
    uint32_t limit_low                  : 16;
    uint32_t base_low                   : 24;
    uint32_t accessed                   : 1;
    uint32_t read_write                 : 1;
    uint32_t direction_conforming       : 1;
    uint32_t executable                 : 1;
    uint32_t descriptor_type            : 1;
    uint32_t descriptor_privilage_level : 2;
    uint32_t present                    : 1;
    uint32_t limit_high                 : 4;
    uint32_t reserved                   : 1; // available?
    uint32_t long_mode                  : 1;
    uint32_t size                       : 1;
    uint32_t granularity                : 1;
    uint32_t base_high                  : 8;
};

struct __attribute__((__packed__)) _idt_ptr {
    uint16_t length;
    uint32_t base;
};

struct __attribute__((__packed__)) _interrupt_descriptor {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t _reserved;
    uint8_t attributes;
    uint16_t offset_high;
};

struct __attribute__((__packed__)) _task_state_segment {
    uint16_t link_old_tss;
    uint16_t reserved1;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved2;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved3;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved4;
    uint32_t reserved5;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t reserved6;
    uint16_t cs;
    uint16_t reserved7;
    uint16_t ss;
    uint16_t reserved8;
    uint16_t ds;
    uint16_t reserved9;
    uint16_t fs;
    uint16_t reserved10;
    uint16_t gs;
    uint16_t reserved11;
    uint16_t ldt;
    uint16_t reserved12;
    uint16_t trap       : 1;
    uint16_t reserved13 : 15;
    uint16_t io_map_base;
};

struct __attribute__((__packed__)) _interrupt_frame {
    // TODO: implement
};

/**
 * Declarations
*/

void __enable_interrupts(void);
void __disable_interrupts(void);
void __init_gdt(uint16_t ss0, uint32_t esp0);
void __set_kernel_stack(uint32_t stack);
void __init_idt(INTERRUPT_DESCRIPTOR *interrupt_descriptor_table, void (*default_isr)(INTERRUPT_FRAME *frame));
void __set_handler(uint8_t irq, uint16_t selector, uint8_t attributes, void (*isr)(INTERRUPT_FRAME *frame));
void __init_tick_counter(void);
void __switch_task(uint32_t eip);
__attribute__((interrupt)) void __update_tick_counter(INTERRUPT_FRAME *frame);
uint64_t __current_tick_count(void);
void __delay_ms(uint32_t ms);