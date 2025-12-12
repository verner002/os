/**
 * @file cpu.h
 * @author verner002
 * @date 22/11/2025
*/

#pragma once

#include "drivers/cpu.h"
#include "drivers/8259a.h"
#include "mm/e820.h"
#include "kernel/ts.h"
#include "kstdlib/string.h"
#include "kstdlib/stdio.h"

#define IDT_ENTRY_PRESENT           0x80
#define IDT_32BIT_INTERRUPT_ENTRY   0x0e

struct __attribute__((__packed__)) __interrupt_frame {
    // TODO: implement
};


void __enable_interrupts(void);
void __disable_interrupts(void);
uint32_t __cpuid_features(void);
void __gdt_init(uint16_t ss0/*, uint32_t esp0*/);
void __gdt_set_kstack(uint32_t stack);
void __idt_init(void);
void __idt_set_handler(uint8_t irq, uint16_t selector, uint8_t attributes, void (*isr)(struct __interrupt_frame *frame));
void __init_tick_counter(void);
void __update_tick_counter(void);
uint64_t __current_tick_count(void);
void __delay_ms(uint32_t ms);