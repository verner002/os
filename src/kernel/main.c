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

static int __compare_ards(void const *ard1, void const *ard2) {
    return (((ADDRESS_RANGE_DESCRIPTOR *)ard1)->base > ((ADDRESS_RANGE_DESCRIPTOR *)ard2)->base) - (((ADDRESS_RANGE_DESCRIPTOR *)ard1)->base < ((ADDRESS_RANGE_DESCRIPTOR *)ard2)->base);
}

static void __default_isr(void) {
    printf("[Interrupt]\n");
}

/**
 * entry
*/

void entry(dword smap_ards_count, ADDRESS_RANGE_DESCRIPTOR *smap, dword cursor_y, dword cursor_x) {
    __setcurpos(cursor_y, cursor_x);
    
    printf("Welcome to Kernel!\n");

    //printf("__default_isr: %p\n", &__default_isr);
    printf("Initializing IDT... ");
    errno = 0; // reset errno
    __init_idt((INTERRUPT_DESCRIPTOR *)0x00007000, &__default_isr);
    printf("%s\n", errno ? "Error" : "Ok");

    /*__init_tick_counter();
    __set_handler(0x00, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__update_tick_counter);*/

    printf("Initializing PICs... ");
    errno = 0; // reset errno
    __init_pics(0x0008, 0x0070); // irqs 0-f -> int 20->2f, sets errno
    __disable_irqs();
    printf("%s\n", errno ? "Error" : "Ok");

    printf("Initializing PIT... ");
    errno = 0; // reset errno
    __init_pit();
    // init ticks counter
    //__outw(PIT_CHANNEL_0_DATA_REGISTER, 0x001234de / 1000); // channel 0 freq=1kHz
    printf("%s\n", errno ? "Error" : "Ok");

    printf("Initializing PS/2... ");
    errno = 0; // reset errno
    __init_ps2();
    printf("%s\n", errno ? "Error" : "Ok");

    __enable_interrupts();
    //__enable_irqs();

    printf("Initializing FDC...\n");
    printf(" Detecting FDDs... ");
    errno = 0; // reset errno
    __init_drives();
    if (!errno) {
        __set_handler(0x0e, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__fdc_irq6_handler);
        __enable_irq(0x06); // irq6
        printf("Ok\n Software reset... ");
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
        
        printf("[BIOS-e820]: %p - %p, type: %s\n", (void *)descriptor->base, (void *)(descriptor->base + descriptor->size - 1), type);
    }

    // !!! VALUES USED ARE ONLY FOR TESTING !!!
    __init_pager((dword *)0x00008000, 32); // TODO: use smap to find suitable region for pmm?
    printf("physical page address: %p\n", pgalloc()); // test 1 (ok)
    printf("physical page address: %p\n", pgalloc()); // test 2 (fail)
    pgfree((void *)4096);
    printf("physical page address: %p\n", pgalloc()); // test 3 (ok)

    for (;;) {
        asm("cli");
        asm("hlt");
    }
}