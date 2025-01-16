/**
 * Kernel
 * 
 * Author: verner002
*/

//#define __DEBUG

/**
 * Includes
*/

#include "null.h"
#include "kstdlib/errno.h"
#include "kernel/syms.h"
#include "kernel/e820.h"
#include "kernel/pager.h"
#include "kernel/userland.h"
#include "kernel/task.h"

#include "drivers/cpu.h"
#include "drivers/acpi.h"
#include "drivers/8259a.h"
#include "drivers/8254a.h"
#include "drivers/8042.h"
#include "drivers/8237a.h"
#include "drivers/82077aa.h"

#include "kstdlib/stdio.h"
#include "kstdlib/stdlib.h"

/**
 * Global Variables
*/

TASK *current_task = NULL;

/**
 * panic
*/

void panic(void) {
    printk("\033[31mKERNEL PANIC\n");

    for (;;) {
        asm("cli");
        asm("hlt");
    }
}

__attribute__((interrupt)) static void __default_isr(INTERRUPT_FRAME *frame) {
    printk("warning: unhandled interrupt!\n");
    for (;;);
}

__attribute__((interrupt)) static void __division_by_zero(INTERRUPT_FRAME *frame) {
    printk("fault: division by zero!\n");
    for (;;);
}

__attribute__((interrupt)) static void __nmi(INTERRUPT_FRAME *frame) {
    printk("info: nmi!\n");
    for (;;);
}

__attribute__((interrupt)) static void __overflow(INTERRUPT_FRAME *frame) {
    printk("trap: overflow!\n");
    for (;;);
}

__attribute__((interrupt)) static void __bounds_check(INTERRUPT_FRAME *frame) {
    printk("fault: bounds check!\n");
    for (;;);
}

__attribute__((interrupt)) static void __invalid_opcode(INTERRUPT_FRAME *frame) {
    printk("fault: invalid opcode!\n");
    for (;;);
}

__attribute__((interrupt)) static void __device_not_available(INTERRUPT_FRAME *frame) {
    printk("fault: device not available!\n");
    for (;;);
}

__attribute__((interrupt)) static void __double_fault(INTERRUPT_FRAME *frame) {
    printk("abort: double fault!\n");
    for (;;);
}

__attribute__((interrupt)) static void __general_protection_fault(INTERRUPT_FRAME *frame) {
    printk("fault: general protection fault!\n");
    for (;;);
}

__attribute__((interrupt)) static void __page_fault(INTERRUPT_FRAME *frame) {
    printk("fault: page fault!\n");
    for (;;);
}

__attribute__((interrupt)) static void __ps2_irq1_handler(INTERRUPT_FRAME *frame) {
    __send_eoi(0x01);
    __inb(PS2_DATA_PORT_REGISTER);
    printk("\033[33mkbd:\033[37m reading buffer\n");
}

__attribute__((interrupt)) static void test(INTERRUPT_FRAME *frame) {
    printk("\033[33mkernel:\033[37m syscall\n");
}

void test_func(void) {
    asm("int 0x80");
}

/**
 * entry
*/

void entry(uint32_t e820_entries_count, E820_ENTRY *e820_entries, void *paging_directory, uint32_t cursor_y, uint32_t cursor_x, SYMBOL *symbol_table, uint32_t symbols_count, char *string_table) {
    __init_vga();
    
    __setcurpos(cursor_y, cursor_x);
    printf("\033[97mWelcome to Kernel!\033[37m\n");

    // reset tick counter
    __init_tick_counter();

    #ifdef __DEBUG
        printk("e820_entries=%p, regions=%u\n", e820_entries, e820_entries_count);
        printk("symbol_table=%p, symbols=%u\n", symbol_table, symbols_count);
        printk("string_table=%p\n", string_table);
    #endif

    printk("\033[33mvga:\033[37m Initialized\n");

    printk("Loading GDT... ");
    __init_gdt(0x10, 0x7c00);
    printf("Ok\n");
    
    printk("Sanitizing E820... ");
    errno = 0;
    __init_e820(e820_entries_count, e820_entries);
    printf("%s\n", errno ? "Error" : "Ok");

    dump_e820();

    E820_ENTRY *first_descriptor = e820_get_descriptor(0);

    if (first_descriptor->type == 1 || first_descriptor->base || first_descriptor->size < 0x00001000) {
        printk("Reserving memory for IVT and BDA... ");

        e820_alloc(1); // TODO: use insert

        first_descriptor = e820_get_descriptor(0);

        if (first_descriptor->type == 2 && !first_descriptor->base && first_descriptor->size >= 0x00001000) printf("Ok\n");
        else {
            printf("Error\n");
            panic();
        }
    }

    printk("\033[33midt:\033[37m Initializing... ");
    // e820 is page-aligned, so we reserve 1 page even though we're going to use only half of it

    INTERRUPT_DESCRIPTOR *idt = (INTERRUPT_DESCRIPTOR *)e820_alloc(1);

    if (!idt) {
        printf("Error\n");
        panic();
    }

    errno = 0; // reset errno
    __init_idt((INTERRUPT_DESCRIPTOR *)0x00007000, &__default_isr); // TODO: use smap to find suitable region for idt?
    __set_handler(0x00, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__division_by_zero);
    __set_handler(0x02, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__nmi);
    __set_handler(0x04, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__overflow);
    __set_handler(0x05, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__bounds_check);
    __set_handler(0x06, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__invalid_opcode);
    __set_handler(0x07, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__device_not_available);
    __set_handler(0x08, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__double_fault);
    __set_handler(0x0d, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__general_protection_fault);
    __set_handler(0x0e, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__page_fault);
    __disable_irqs();
    __enable_interrupts();
    printf("%s\n", errno ? "Error" : "Ok");

    if (__init_acpi()) {
        printk("\033[91mFailed to initialize ACPI\033[37m\n");
        // set mode to 8259
    } // else set mode to acpi (initialize apic)

    printk("Initializing PICs... ");
    errno = 0; // reset errno
    __init_pics(0x0020, 0x0070); // irqs 0-7 -> int 20->27, irqs 8-f -> 70->77, sets errno
    printf("%s\n", errno ? "Error" : "Ok");

    printk("Initializing PIT... ");
    errno = 0; // reset errno
    __init_pit();
    // channel 0 for ticks counter, FIXME: doesn't work in bochs
    __outw(PIT_CHANNEL_0_DATA_REGISTER, 0x001234de / 1000); // channel 0 freq=1kHz
    __disable_interrupts();
    __send_eoi_master();
    __set_handler(0x20, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__update_tick_counter);
    __enable_interrupts();
    __enable_irq(0x00); // irq0
    printf("%s\n", errno ? "Error" : "Ok");

    printk("Initializing PS/2... ");
    errno = 0; // reset errno
    __init_ps2();
    __disable_interrupts();
    __send_eoi_master();
    __set_handler(0x21, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__ps2_irq1_handler);
    __enable_interrupts();
    __enable_irq(0x01); // irq1
    printf("%s\n", errno ? "Error" : "Ok");

    /*printk("Initializing DMA... ");
    __init_dma();
    printf("Ok\n");*/

    if (__init_fdc()) {
        printk("\033[91mFailed to initialize FDC\033[37m\n");
        // ignore?
    }

    printk("Initializing PMM... ");
    // !!! VALUES USED ARE ONLY FOR TESTING !!!
    __init_pager((uint32_t *)0x00008000, 32); // TODO: use smap to find suitable region for pmm?
    printf("Ok\n");

    printk("Initializing heap... ");
    __init_heap(pgalloc(), 4096);
    printf("Ok\n");

    loop:
    goto loop;

    /*printk("waiting 5 seconds...\n");
    __delay_ms(5000);
    printk("done!\n");*/

    /*for (uint32_t i = 0; i < symbols_count; ++i) {
        SYMBOL *symbol = &symbol_table[i];

        printf("symbol %u:\n name (", i);

        if (*((uint32_t *)symbol->symbol_name)) {
            printf("short): ");
            for (uint32_t j = 0; j < 8; ++j) putchar(symbol->symbol_name[j]);
        } else printf("long): %s", string_table + *((uint32_t *)(symbol->symbol_name + 4)));
        
        char *type;

        switch (symbol->symbol_type) {
            case 0: type = "undefined"; break;
            case 1: type = "absolute"; break;
            case 2: type = "debugging"; break;
            default: type = "unknown"; break;
        }

        printf("\n type: %s\n section number: %u\n\n", type, symbol->section_number);
        //__delay_ms(2000);
    }*/

    __set_handler(0x80, 0x0008, 0xee, &test); // FIXME: implement setting dpl, this is stupid

    __disable_interrupts();
    TASK *system = current_task = (TASK *)malloc(sizeof(TASK));
    
    TASK *test = (TASK *)malloc(sizeof(TASK));
    test->id = 1;
    test->eip = (uint32_t)&test_func;
    test->esp = test->ebp = pgalloc() + 4096;
    test->kernel_stack = pgalloc() + 4096;
    test->next = system;

    system->id = 0;
    system->esp = system->ebp = pgalloc() + 4096;
    system->kernel_stack = pgalloc() + 4096;
    system->next = test;
    __enable_interrupts();

    //__switch_task();

    uint32_t eip = __get_eip();
    printk("cycle\n");
    __exec_kernelmode(eip);

    printf("kernel\n");

    // idle loop
    for (;;) {
        asm("hlt");
    }
}