/**
 * CPU
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/cpu.h"
#include "drivers/isr.h"

/**
 * Static Global Variable
*/

static GLOBAL_DESCRIPTOR gdt[6]; // global descriptor table
static INTERRUPT_DESCRIPTOR *idt; // interrupt descriptor table
static TASK_STATE_SEGMENT tss; // task state segment
static uint64_t ticks; // current tick count, 1 tick = 1 ms

void __isr(uint8_t vector, struct __interrupt_frame *frame) {
    printk("isr: %u: unhandled interrupt\n", vector);
    for(;;);
}

/**
 * __enable_interrupts
*/

void __enable_interrupts(void) {
    asm volatile ("sti");
}

/**
 * __disable_iunterrupts
*/

void __disable_interrupts(void) {
    asm volatile ("cli");
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
 * __init_gdt
*/

void __init_gdt(uint16_t ss0, uint32_t esp0) {
    printk("\033[33mcpu:\033[37m Initializing GDT... ");

    GDT_PTR gdt_ptr = {
        .length = sizeof(gdt) - 1,
        .base = (uint32_t)&gdt
    };

    gdt[0] = (GLOBAL_DESCRIPTOR) { // null descriptor
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

    gdt[1] = (GLOBAL_DESCRIPTOR) { // code descriptor
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

    gdt[2] = (GLOBAL_DESCRIPTOR) { // data descriptor
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

    gdt[3] = (GLOBAL_DESCRIPTOR) { // code descriptor
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

    gdt[4] = (GLOBAL_DESCRIPTOR) { // data descriptor
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
    uint32_t size = sizeof(TASK_STATE_SEGMENT);

    gdt[5] = (GLOBAL_DESCRIPTOR) { // tss
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

    tss.ss0 = ss0;
    tss.esp0 = esp0;

    tss.io_map_base = size; // no io bitmap

    tss.cs = 0x000b;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x0013;

    asm volatile (
        "lgdt %0"
        :
        : "m" (gdt_ptr)
        :
    ); // flush gdt

    __flush_tss();
    printf("Ok\n");
}

/**
 * __set_kernel_stack
 * 
 * Note:
 *  Sets a kernel stack for each task.
*/

void __set_kernel_stack(uint32_t stack) {
    tss.esp0 = stack;
}

/**
 * __init_idt
*/

int32_t __init_idt(void (*default_isr)(INTERRUPT_FRAME *frame)) {
    printk("\033[33mcpu:\033[37m Initializing IDT... ");
    
    idt = (INTERRUPT_DESCRIPTOR *)__e820_rmalloc(2048, TRUE);

    if (!idt) {
        printf("Error\n");
        return -1;
    }

    IDT_PTR idt_ptr = {
        .length = 0x07ff, // 256*sizeof(INTERRUPT_DESCRIPTOR)-1
        .base = (uint32_t)idt
    };

    INTERRUPT_DESCRIPTOR __default_descriptor = {
        .offset_low = (uint16_t)((uint32_t)default_isr & 0xffff),
        .selector = 0x0008, // code segment selector
        ._reserved = 0,
        .attributes = INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE,
        .offset_high = (uint16_t)(((uint32_t)default_isr >> 16) & 0xffff)
    };

    /*for (uint32_t i = 0; i < 256; ++i)
        idt[i] = __default_descriptor;*/

    __isr_init();

    asm volatile (
        "lidt %0"
        :
        : "m" (idt_ptr)
        :
    ); // flush idt

    printf("Ok\n");
    return 0;
}

/**
 * __set_handler
*/

void __set_handler(uint8_t irq, uint16_t selector, uint8_t attributes, void (*isr)(INTERRUPT_FRAME *frame)) {
    INTERRUPT_DESCRIPTOR id = {
        .offset_low = (uint16_t)((uint32_t)isr & 0xffff),
        .selector = selector,
        ._reserved = 0,
        .attributes = attributes,
        .offset_high = (uint16_t)(((uint32_t)isr >> 16) & 0xffff)
    };

    idt[irq] = id;
    idt[irq] = id;
    idt[irq] = id;
}

/**
 * __init_tick_counter
*/

void __init_tick_counter(void) {
    ticks = 0;

    // TODO: set pit channel 0 to generate irq
    // TODO: map irq to isr __update_tick_counter
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