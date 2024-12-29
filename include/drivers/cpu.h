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
    word length;
    dword base;
};

struct __attribute__((__packed__)) _global_descriptor {
    unsigned int limit_low                  : 16;
    unsigned int base_low                   : 24;
    unsigned int accessed                   : 1;
    unsigned int read_write                 : 1;
    unsigned int direction_conforming       : 1;
    unsigned int executable                 : 1;
    unsigned int descriptor_type            : 1;
    unsigned int descriptor_privilage_level : 2;
    unsigned int present                    : 1;
    unsigned int limit_high                 : 4;
    unsigned int reserved                   : 1; // available?
    unsigned int long_mode                  : 1;
    unsigned int size                       : 1;
    unsigned int granularity                : 1;
    unsigned int base_high                  : 8;
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

struct __attribute__((__packed__)) _task_state_segment {
    word link_old_tss;
    word reserved1;
    dword esp0;
    word ss0;
    word reserved2;
    dword esp1;
    word ss1;
    word reserved3;
    dword esp2;
    word ss2;
    word reserved4;
    dword reserved5;
    dword eip;
    dword eflags;
    dword eax;
    dword ecx;
    dword edx;
    dword ebx;
    dword esp;
    dword ebp;
    dword esi;
    dword edi;
    word es;
    word reserved6;
    word cs;
    word reserved7;
    word ss;
    word reserved8;
    word ds;
    word reserved9;
    word fs;
    word reserved10;
    word gs;
    word reserved11;
    word ldt;
    word reserved12;
    unsigned short trap       : 1;
    unsigned short reserved13 : 15;
    word io_map_base;
};

struct __attribute__((__packed__)) _interrupt_frame {
    // TODO: implement
};

/**
 * Declarations
*/

void __enable_interrupts(void);
void __disable_interrupts(void);
void __init_gdt(unsigned short ss0, unsigned int esp0);
void __set_kernel_stack(unsigned int stack);
void __init_idt(INTERRUPT_DESCRIPTOR *interrupt_descriptor_table, void (*default_isr)(INTERRUPT_FRAME *frame));
void __set_handler(byte irq, word selector, byte attributes, void (*isr)(INTERRUPT_FRAME *frame));
void __init_tick_counter(void);
void __switch_task(void);
__attribute__((interrupt)) void __update_tick_counter(INTERRUPT_FRAME *frame);
unsigned long __current_tick_count(void);
void __delay_ms(unsigned int ms);