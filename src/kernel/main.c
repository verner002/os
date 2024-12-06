/**
 * Kernel
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "null.h"
#include "kstdlib/errno.h"
#include "kernel/syms.h"
#include "kernel/e820.h"
#include "kernel/pager.h"

#include "drivers/cpu.h"
#include "drivers/8259a.h"
#include "drivers/8254a.h"
#include "drivers/8042.h"
#include "drivers/82077aa.h"

#include "kstdlib/stdio.h"
#include "kstdlib/stdlib.h"

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

/**
 * entry
*/

void entry(unsigned int ards_count, E820_ENTRY *ard_table, dword cursor_y, dword cursor_x, SYMBOL *symbol_table, unsigned int symbols_count, char *string_table) {
    __init_vga();
    
    __setcurpos(cursor_y, cursor_x);
    printf("\033[97mWelcome to Kernel!\033[37m\n");

    // reset tick counter
    __init_tick_counter();

    printk("e820_table=%p, entries=%u\n", ard_table, ards_count);
    printk("symbol_table=%p, entries=%u\n", symbol_table, symbols_count);
    printk("string_table=%p\n", string_table);

    //dump_e820(ards_count, ard_table);
    
    printk("Sanitizing E820... ");
    errno = 0;
    E820_MAP *smap = __sanitize_e820(ards_count, ard_table);
    printf("%s\n", errno ? "Error" : "Ok");
    // TODO: allocate memory for idt after sanitizing e820

    dump_e820(ards_count, ard_table);
    //dump_e820(smap->index, smap->entries);

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
    __set_handler(0x08, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__double_fault);
    __set_handler(0x0d, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__general_protection_fault);
    __set_handler(0x0e, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__page_fault);
    __disable_irqs();
    __enable_interrupts();
    printf("%s\n", errno ? "Error" : "Ok");

    printk("Initializing PICs... ");
    errno = 0; // reset errno
    __init_pics(0x0020, 0x0070); // irqs 0-7 -> int 20->27, irqs 8-f -> 70->77, sets errno
    printf("%s\n", errno ? "Error" : "Ok");

    printk("Initializing PIT... ");
    errno = 0; // reset errno
    __init_pit();
    // channel 0 for ticks counter, FIXME: doesn't work in bochs
    __outw(PIT_CHANNEL_0_DATA_REGISTER, 0x001234de / 1000); // channel 0 freq=1kHz
    __set_handler(0x20, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__update_tick_counter);
    __enable_irq(0x00); // irq0
    printf("%s\n", errno ? "Error" : "Ok");

    printk("Initializing PS/2... ");
    errno = 0; // reset errno
    __init_ps2();
    __set_handler(0x21, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__ps2_irq1_handler);
    __enable_irq(0x01); // irq1
    printf("%s\n", errno ? "Error" : "Ok");

    printk("Initializing FDC...\n");
    printk("\033[33mfdc:\033[37m Detecting FDDs... ");
    errno = 0; // reset errno
    __init_drives();
    if (!errno) {
        __set_handler(0x26, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__fdc_irq6_handler);
        __enable_irq(0x06); // irq6
        printf("Ok\n");
        printk("\033[33mfdc:\033[37m Software reset... ");
        __software_reset();
        printf("%s\n", errno ? "Error" : "Ok");
    } else printf("Error\n");

    printk("Initializing PMM... ");
    // !!! VALUES USED ARE ONLY FOR TESTING !!!
    __init_pager((dword *)0x00008000, 32); // TODO: use smap to find suitable region for pmm?
    printf("Ok\n");

    void *my_page;
    printk("pgalloc: %p\n", pgalloc()); // test 1 (ok)
    printk("pgalloc: %p\n", (my_page = pgalloc())); // test 2 (ok)
    printk("pgfree: %p\n", my_page);
    pgfree(my_page);
    printk("pgalloc: %p\n", pgalloc()); // test 3 (fail)

    /*printk("waiting 5 seconds...\n");
    __delay_ms(5000);
    printk("done!\n");*/

    /*for (unsigned int i = 0; i < symbols_count; ++i) {
        SYMBOL *symbol = &symbol_table[i];

        printf("symbol %u:\n name (", i);

        if (*((unsigned int *)symbol->symbol_name)) {
            printf("short): ");
            for (unsigned int j = 0; j < 8; ++j) putchar(symbol->symbol_name[j]);
        } else printf("long): %s", string_table + *((unsigned int *)(symbol->symbol_name + 4)));
        
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

    for (;;) {
        /*asm("cli");
        asm("hlt");*/
    }
}