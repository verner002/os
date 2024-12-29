/**
 * CPU
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/cpu.h"

/**
 * Static Global Variable
*/

static GLOBAL_DESCRIPTOR gdt[6]; // global descriptor table
static INTERRUPT_DESCRIPTOR *idt; // interrupt descriptor table
static TASK_STATE_SEGMENT tss; // task state segment
static unsigned long ticks; // current tick count, 1 tick = 1 ms

/**
 * __enable_interrupts
*/

void __enable_interrupts(void) {
    asm("sti");
}

/**
 * __disable_iunterrupts
*/

void __disable_interrupts(void) {
    asm("cli");
}

/**
 * __init_gdt
*/

void __init_gdt(unsigned short ss0, unsigned int esp0) {
    GDT_PTR gdt_ptr = {
        .length = sizeof(gdt) - 1,
        .base = (dword)&gdt
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

    dword base = (dword)&tss;
    dword size = sizeof(TASK_STATE_SEGMENT);

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

    tss.cs = 0x0b;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;

    asm (
        "lgdt %0"
        :
        : "m" (gdt_ptr)
        :
    );

    __flush_tss();
}

void __set_kernel_stack(unsigned int stack) {
    tss.esp0 = stack;
}

/**
 * __init_idt
*/

void __init_idt(INTERRUPT_DESCRIPTOR *interrupt_descriptor_table, void (*default_isr)(INTERRUPT_FRAME *frame)) {
    idt = interrupt_descriptor_table;

    IDT_PTR idt_ptr = {
        .length = 0x07ff,
        .base = (dword)idt
    };

    INTERRUPT_DESCRIPTOR __default_descriptor = {
        .offset_low = (word)((dword)default_isr & 0xffff),
        .selector = 0x0008, // code segment selector
        .attributes = INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE,
        .offset_high = (word)(((dword)default_isr >> 16) & 0xffff)
    };

    for (unsigned int i = 0; i < 256; ++i) idt[i] = __default_descriptor;

    asm (
        "lidt %0"
        :
        : "m" (idt_ptr)
        :
    );
}

/**
 * __set_handler
*/

void __set_handler(byte irq, word selector, byte attributes, void (*isr)(INTERRUPT_FRAME *frame)) {
    INTERRUPT_DESCRIPTOR id = {
        .offset_low = (word)((dword)isr & 0xffff),
        .selector = selector,
        .attributes = attributes,
        .offset_high = (word)(((dword)isr >> 16) & 0xffff)
    };

    idt[irq] = id;
}

/**
 * __init_tick_counter
*/

void __init_tick_counter(void) {
    ticks = 0;

    // set pit channel 0 to generate irq
    // map irq to isr __update_tick_counter
}

extern TASK *current_task;

/**
 * __switch_task
*/

void __switch_task(void) {
    if (!current_task) return; // not initialized yet

    __disable_interrupts();

    unsigned int esp, ebp, eip;

    asm volatile (
        "mov esp, %0\t\n"
        "mov ebp, %1\t\n"
        : "=r" (esp), "=r" (ebp)
        :
        :
    );

    
}

__attribute__((interrupt)) void __update_tick_counter(INTERRUPT_FRAME *frame) {
    __disable_interrupts();
    
    unsigned int eip;

    asm (
        "mov eax, dword [ebp+8]"
        : "=a" (eip)
        :
        :
    );
    
    __send_eoi(0x00);

    ++ticks;

    if (current_task) {
        current_task->eip = eip;
        current_task = current_task->next;

        __set_kernel_stack(0);

        if (!current_task->id) {
            printk("switching to task with pid %u, eip=%p\n", current_task->id, eip);
            __exec_kernelmode(current_task->eip);
        } else {
            // usermode
            __exec_usermode(0);
        }
    }
}

/**
 * __current_tick_count
*/

unsigned long __current_tick_count(void) {
    return ticks;
}

/**
 * __sleep_ms
 * 
 * Note:
 *  Another task is begin processed.
 *  Sleeps the task for a given period of time.
 *  (Remove task from queue and let timer fire an
 *  interrupt after a given time to return it?)
*/

/**
 * __delay_ms
 * 
 * Note:
 *  The same task is begin processed.
*/

void __delay_ms(unsigned int ms) {
    unsigned long ticks_end = ticks + ms;

    while (ticks < ticks_end);
}