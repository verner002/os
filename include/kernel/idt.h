/**
 * @file idt.h
 * @author verner002
 * @date 23/04/2026
*/

#pragma once

#include "kernel/types.h"

struct __attribute__((packed)) idt_ptr {
    uint16_t length;
    uint32_t base;
};

struct __attribute__((packed)) idt {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t reserved1;
    uint8_t attributes;
    uint16_t offset_high;
};

struct __attribute__((packed)) interrupt_frame {
    uint32_t ebp;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};

extern volatile struct idt idt[256];

void idt_init(void);

void install_interrupt_gate(
    uint8_t interrupt,
    void (*routine)(struct interrupt_frame *frame)
);

void install_user_interrupt_gate(
    uint8_t interrupt,
    void (*routine)(struct interrupt_frame *frame)
);

void install_trap_gate(
    uint8_t interrupt,
    void (*routine)(struct interrupt_frame *frame)
);