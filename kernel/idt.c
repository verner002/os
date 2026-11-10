/**
 * @file idt.c
 * @author verner002
 * @date 23/04/2026
*/

#define IDT_ENTRY_PRESENT               0x80
#define IDT_USER                        0x60
#define IDT_32BIT_INTERRUPT_ENTRY       0x0e
#define IDT_32BIT_TRAP_ENTRY            0x0f

#include "kernel/idt.h"

// interrupt descriptor table
volatile struct idt idt[256];
volatile struct idt_ptr idt_ptr;

void idt_init(void) {
    // TODO: install default interrupt handlers

    idt_ptr.length = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt;

    asm volatile (
        "lidt %0"
        :
        : "m" (idt_ptr)
        :
    ); // flush idt
}

void install_interrupt_gate(
    uint8_t interrupt,
    void (*routine)(struct interrupt_frame *frame)
) {
    idt[interrupt] = (struct idt){
        .offset_low = (uint16_t)((uint32_t)routine & 0xffff),
        .selector = 0x0008,
        .reserved1 = 0,
        .attributes = IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY,
        .offset_high = (uint16_t)((uint32_t)routine >> 16)
    };
}

void install_user_interrupt_gate(
    uint8_t interrupt,
    void (*routine)(struct interrupt_frame *frame)
) {
    idt[interrupt] = (struct idt){
        .offset_low = (uint16_t)((uint32_t)routine & 0xffff),
        .selector = 0x0008,
        .reserved1 = 0,
        .attributes = IDT_ENTRY_PRESENT | IDT_USER | IDT_32BIT_INTERRUPT_ENTRY,
        .offset_high = (uint16_t)((uint32_t)routine >> 16)
    };
}

void install_trap_gate(
    uint8_t interrupt,
    void (*routine)(struct interrupt_frame *frame)
) {
    idt[interrupt] = (struct idt){
        .offset_low = (uint16_t)((uint32_t)routine & 0xffff),
        .selector = 0x0008,
        .reserved1 = 0,
        .attributes = IDT_ENTRY_PRESENT | IDT_32BIT_TRAP_ENTRY,
        .offset_high = (uint16_t)((uint32_t)routine >> 16)
    };
}