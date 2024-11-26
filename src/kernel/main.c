/**
 * Kernel
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "null.h"
#include "kernel/smap.h"
#include "kernel/pager.h"
#include "drivers/8254a.h"
#include "kstdlib/stdio.h"
#include "kstdlib/stdlib.h"

/**
 * __compare_ards
*/

static int __compare_ards(void const *ard1, void const *ard2) {
    return (((ADDRESS_RANGE_DESCRIPTOR *)ard1)->base > ((ADDRESS_RANGE_DESCRIPTOR *)ard2)->base) - (((ADDRESS_RANGE_DESCRIPTOR *)ard1)->base < ((ADDRESS_RANGE_DESCRIPTOR *)ard2)->base);
}

/**
 * entry
*/

void entry(dword smap_ards_count, ADDRESS_RANGE_DESCRIPTOR *smap, dword cursor_y, dword cursor_x) {
    __setcurpos(cursor_y, cursor_x);
    
    printf("Welcome to Kernel!\n");

    // init ticks counter
    //__init_pit();
    //__outw(PIT_CHANNEL_0_DATA_REGISTER, 0x001234de / 1000); // channel 0 freq=1kHz

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
        
        printf("base: %p, size: %u, type: %s\n", (void *)descriptor->base, descriptor->size, type);
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