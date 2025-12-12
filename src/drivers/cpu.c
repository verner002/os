/**
 * @file cpu.c
 * @author verner002
 * @date 22/11/2025
*/

#include "drivers/cpu.h"
#include "drivers/isr.h"

struct __attribute__((__packed__)) __gdt_ptr {
    uint16_t length;
    uint32_t base;
};

struct __attribute__((__packed__)) __idt_ptr {
    uint16_t length;
    uint32_t base;
};

struct __attribute__((__packed__)) __global_descriptor {
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

struct __attribute__((__packed__)) __interrupt_descriptor {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t _reserved;
    uint8_t attributes;
    uint16_t offset_high;
};

struct __attribute__((__packed__)) __tss {
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
    uint16_t trap           : 1;
    uint16_t reserved13     : 15;
    uint16_t io_map_base;
};

static struct __global_descriptor gdt[6]; // global descriptor table
static struct __interrupt_descriptor idt[256]; // interrupt descriptor table
static struct  __tss tss; // task state segment
static uint64_t ticks; // current tick count, 1 tick = 1 ms

/**
 * __isr
*/

void __isr(uint8_t vector, struct __interrupt_frame *frame) {
    printk("isr: %u: unhandled interrupt\n", vector);
    for(;;);
}

/**
 * __disable_iunterrupts
*/

void __disable_interrupts(void) {
    asm volatile ("cli");
}

/**
 * __enable_interrupts
*/

void __enable_interrupts(void) {
    asm volatile ("sti");
}

/**
 * __cpuid_features
*/

uint32_t __cpuid_features(void) {
    uint32_t features;

    asm volatile (
        "mov eax, 0x00000001\n\t"
        "cpuid\n\t"
        : "=d" (features)
        :
        :
    );

    return features;
}

/**
 * __gdt_init
*/

void __gdt_init(uint16_t ss0/*, uint32_t esp0*/) {
    gdt[0] = (struct __global_descriptor) { // null descriptor
        .limit_low = 0,
        .base_low = 0,
        .accessed = 0,
        .read_write = 0,
        .direction_conforming = 0,
        .executable = 0,
        .descriptor_type = 0,
        .descriptor_privilage_level = 0,
        .present = 0,
        .limit_high = 0,
        .reserved = 0,
        .long_mode = 0,
        .size = 0,
        .granularity = 0,
        .base_high = 0
    };

    gdt[1] = (struct __global_descriptor) { // code descriptor
        .limit_low = 0xffff,
        .base_low = 0x000000,
        .accessed = 0,
        .read_write = 1,
        .direction_conforming = 0,
        .executable = 1,
        .descriptor_type = 1,
        .descriptor_privilage_level = 0,
        .present = 1,
        .limit_high = 0xf,
        .reserved = 0,
        .long_mode = 0,
        .size = 1,
        .granularity = 1,
        .base_high = 0x00
    };

    gdt[2] = (struct __global_descriptor) { // data descriptor
        .limit_low = 0xffff,
        .base_low = 0x000000,
        .accessed = 0,
        .read_write = 1,
        .direction_conforming = 0,
        .executable = 0,
        .descriptor_type = 1,
        .descriptor_privilage_level = 0,
        .present = 1,
        .limit_high = 0xf,
        .reserved = 0,
        .long_mode = 0,
        .size = 1,
        .granularity = 1,
        .base_high = 0x00
    };

    gdt[3] = (struct __global_descriptor) { // code descriptor
        .limit_low = 0xffff,
        .base_low = 0x000000,
        .accessed = 0,
        .read_write = 1,
        .direction_conforming = 0,
        .executable = 1,
        .descriptor_type = 1,
        .descriptor_privilage_level = 3,
        .present = 1,
        .limit_high = 0xf,
        .reserved = 0,
        .long_mode = 0,
        .size = 1,
        .granularity = 1,
        .base_high = 0x00
    };

    gdt[4] = (struct __global_descriptor) { // data descriptor
        .limit_low = 0xffff,
        .base_low = 0x000000,
        .accessed = 0,
        .read_write = 1,
        .direction_conforming = 0,
        .executable = 0,
        .descriptor_type = 1,
        .descriptor_privilage_level = 3,
        .present = 1,
        .limit_high = 0xf,
        .reserved = 0,
        .long_mode = 0,
        .size = 1,
        .granularity = 1,
        .base_high = 0x00
    };

    uint32_t base = (uint32_t)&tss;
    uint32_t size = sizeof(struct __tss);

    gdt[5] = (struct __global_descriptor) { // tss
        .limit_low = size,
        .base_low = base,
        .accessed = 1,
        .read_write = 0,
        .direction_conforming = 0,
        .executable = 1,
        .descriptor_type = 0,
        .descriptor_privilage_level = 0,
        .present = 1,
        .limit_high = (size & 0x000f0000) >> 16,
        .reserved = 0,
        .long_mode = 0,
        .size = 0,
        .granularity = 0,
        .base_high = (base & 0xff000000) >> 24
    };

    memset(&tss, 0, size);

    // kernel stack segment and pointer
    tss.ss0 = ss0;
    //tss.esp0 = esp0;

    // no io bitmap
    tss.io_map_base = size;

    // user code and data segments
    tss.cs = 0x000b;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x0013;

    struct __gdt_ptr gdt_ptr = {
        .length = sizeof(gdt) - 1,
        .base = (uint32_t)&gdt
    };

    asm volatile (
        "lgdt %0"
        :
        : "m" (gdt_ptr)
        :
    ); // flush gdt

    __flush_tss();
}

/**
 * __gdt_set_kstack
*/

void __gdt_set_kstack(uint32_t stack) {
    tss.esp0 = stack;
}

/**
 * __idt_init
*/

void __idt_init(void) {
    __isr_init();

    struct __idt_ptr idt_ptr = {
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
    struct __interrupt_descriptor id = {
        .offset_low = (uint16_t)((uint32_t)isr & 0xffff),
        .selector = selector,
        ._reserved = 0,
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