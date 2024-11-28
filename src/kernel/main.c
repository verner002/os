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
#include "kernel/smap.h"
#include "kernel/syms.h"
#include "kernel/pager.h"

#include "drivers/cpu.h"
#include "drivers/8259a.h"
#include "drivers/8254a.h"
#include "drivers/8042.h"
#include "drivers/82077aa.h"

#include "kstdlib/stdio.h"
#include "kstdlib/stdlib.h"

/**
 * __compare_ards
*/

__attribute__((interrupt)) static void __default_isr(INTERRUPT_FRAME *frame) {
    fprintf(stderr, "[%s]: Unhandled interrupt!\n", __func__);
}

__attribute__((interrupt)) static void __division_by_zero(INTERRUPT_FRAME *frame) {
    fprintf(stderr, "[%s]: Unhandled interrupt!\n", __func__);
}

__attribute__((interrupt)) static void __double_fault(INTERRUPT_FRAME *frame) {
    fprintf(stderr, "[%s]: Unhandled interrupt!\n", __func__);
}

__attribute__((interrupt)) static void __general_protection_fault(INTERRUPT_FRAME *frame) {
    fprintf(stderr, "[%s]: Unhandled interrupt!\n", __func__);
    for (;;);
}

__attribute__((interrupt)) static void __page_fault(INTERRUPT_FRAME *frame) {
    fprintf(stderr, "[%s]: Unhandled interrupt!\n", __func__);
}

__attribute__((interrupt)) static void __ps2_irq1_handler(INTERRUPT_FRAME *frame) {
    __send_eoi(0x01);
    __inb(PS2_DATA_PORT_REGISTER);
    putchar('!');
}

static int __compare_ards(void const *ard1, void const *ard2) {
    return (((ADDRESS_RANGE_DESCRIPTOR *)ard1)->base > ((ADDRESS_RANGE_DESCRIPTOR *)ard2)->base) - (((ADDRESS_RANGE_DESCRIPTOR *)ard1)->base < ((ADDRESS_RANGE_DESCRIPTOR *)ard2)->base);
}

static unsigned int log10(unsigned int i) {
    byte digits[] = { 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10 };
    dword powers[] = { 0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
    unsigned int l;

    asm (
        "push eax\t\n"
        "bsr eax, ebx\t\n"
        "movzx eax, BYTE PTR [ecx+eax+1]\t\n"
        "cmp ebx, DWORD PTR [edx+eax*4]\t\n"
        "sbb al, 0\t\n"
        "mov %0, eax\t\n"
        "pop eax"
        : "=m" (l)
        : "b" (i), "c" (digits), "d" (powers)
        :
    );

    return l;
}

static void __log(char const *str) {
    unsigned long ticks = __current_tick_count();
    unsigned long s = ticks / 1000;
    unsigned int ms = ticks % 1000;

    putchar('[');

    unsigned int padding = 6 - log10(s);
    for (unsigned int i = 0; i < padding; ++i) putchar(' ');
    printf("%u.", (unsigned int)s);

    padding = 2 - log10(ms);
    for (unsigned int i = 0; i < padding; ++i) putchar('0');
    printf("%u] %s", ms, str);
}

/**
 * entry
*/

void entry(dword smap_ards_count, ADDRESS_RANGE_DESCRIPTOR *smap, dword cursor_y, dword cursor_x, SYMBOL *symbol_table, unsigned int symbols_count, char *string_table) {
    __setcurpos(cursor_y, cursor_x);
    printf("Welcome to Kernel!\n");

    printf("symbol_table=%p,symbols_count=%u\n", symbol_table, symbols_count);
    printf("string_table=%p\n", string_table);

    // reset tick counter
    __init_tick_counter();

    //printf("__default_isr: %p\n", &__default_isr);
    __log("Initializing IDT... ");
    errno = 0; // reset errno
    __init_idt((INTERRUPT_DESCRIPTOR *)0x00007000, &__default_isr);
    printf("%s\n", errno ? "Error" : "Ok");

    __log("Initializing PICs... ");
    errno = 0; // reset errno
    __init_pics(0x0020, 0x0070); // irqs 0-f -> int 20->2f, sets errno
    __disable_irqs();
    printf("%s\n", errno ? "Error" : "Ok");

    __log("Initializing PIT... ");
    errno = 0; // reset errno
    __init_pit();
    // channel 0 for ticks counter
    __outw(PIT_CHANNEL_0_DATA_REGISTER, 0x001234de / 1000); // channel 0 freq=1kHz
    __set_handler(0x20, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__update_tick_counter);
    __enable_irq(0x00); // irq0
    printf("%s\n", errno ? "Error" : "Ok");

    __log("Initializing PS/2... ");
    errno = 0; // reset errno
    __init_ps2();
    __set_handler(0x21, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__ps2_irq1_handler);
    printf("%s\n", errno ? "Error" : "Ok");

    __set_handler(0x00, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__division_by_zero);
    __set_handler(0x08, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__double_fault);
    __set_handler(0x0d, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__general_protection_fault);
    __set_handler(0x0e, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__page_fault);
    __enable_interrupts();

    __log("Initializing FDC...\n");
    __log("[fdc]: Detecting FDDs... ");
    errno = 0; // reset errno
    __init_drives();
    if (!errno) {
        __set_handler(0x26, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__fdc_irq6_handler);
        __enable_irq(0x06); // irq6
        printf("Ok\n");
        __log("[fdc]: Software reset... ");
        __software_reset();
        printf("%s\n", errno ? "Error" : "Ok");
    } else printf("Error\n");

    // smap may be unsorted and some regions may overlap (in some rare cases, mask as unknown?)
    // fix regions that are not page aligned
    // sort map, *merge* free memory regions (and correct overlapping regions?)
    qsort(smap, smap_ards_count, sizeof(ADDRESS_RANGE_DESCRIPTOR), &__compare_ards);

    for (unsigned int i = 0; i < smap_ards_count; ++i) {
        ADDRESS_RANGE_DESCRIPTOR *descriptor = &smap[i];

        char *type;

        switch (descriptor->type) {
            case 1: type = "Free"; break; // usable memory, merge these regions if possible
            case 2: type = "Reserved"; break; // don't use
            case 3: type = "ACPI Reclaimable"; break; // we can use this after we're done with acpi
            case 4: type = "ACPI NVS"; break; // don't use
            case 5: type = "Bad"; break; // don't use
            default: type = "Unknown"; break; // // don't use, change type to reserved?
        }
        
        __log("[bios-e820]: ");
        printf("%p - %p, type: %s\n", (void *)descriptor->base, (void *)(descriptor->base + descriptor->size - 1), type);
    }

    // !!! VALUES USED ARE ONLY FOR TESTING !!!
    __init_pager((dword *)0x00008000, 32); // TODO: use smap to find suitable region for pmm?
    printf("physical page address: %p\n", pgalloc()); // test 1 (ok)
    printf("physical page address: %p\n", pgalloc()); // test 2 (fail)
    pgfree((void *)4096);
    printf("physical page address: %p\n", pgalloc()); // test 3 (ok)

    /*printf("waiting 5 seconds... ");
    __delay_ms(5000);
    printf("done!\n");*/

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