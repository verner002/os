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
#include "kstdlib/stdio.h"
#include "kstdlib/stdlib.h"

static int compare_ards(void const *ard1, void const *ard2) {
    return (((ADDRESS_RANGE_DESCRIPTOR *)ard1)->size > ((ADDRESS_RANGE_DESCRIPTOR *)ard2)->size) - (((ADDRESS_RANGE_DESCRIPTOR *)ard1)->size < ((ADDRESS_RANGE_DESCRIPTOR *)ard2)->size);
}

static int ci(void const *e1, void const *e2) {
    return (*(int *)e1 > *(int *)e2) - (*(int *)e1 < *(int *)e2);
}

/**
 * entry
*/

void entry(dword smap_ards_count, ADDRESS_RANGE_DESCRIPTOR *smap, dword cursor_y, dword cursor_x) {
    __setcurpos(cursor_y, cursor_x);
    
    printf("Kernel ready!\n");

    for (unsigned int i = 0; i < smap_ards_count; ++i) { // sort map and merge free, reserved and overlapping regions
        ADDRESS_RANGE_DESCRIPTOR *descriptor = &smap[i];

        char *type;

        switch (descriptor->type) {
            case 1: type = "Free"; break;
            case 2: type = "Reserved"; break;
            case 3: type = "ACPI Reclaimable"; break;
            case 4: type = "NVS"; break;
            case 5: type = "Bad"; break;
            default: type = "Unknown"; break;
        }
        
        printf("base: %u, size: %u, type: %s\n", descriptor->base, descriptor->size, type);
    }

    putchar('\n');

    qsort(smap, smap_ards_count, sizeof(ADDRESS_RANGE_DESCRIPTOR), &compare_ards);

    for (unsigned int i = 0; i < smap_ards_count; ++i) { // sort map and merge free, reserved and overlapping regions
        ADDRESS_RANGE_DESCRIPTOR *descriptor = &smap[i];

        char *type;

        switch (descriptor->type) {
            case 1: type = "Free"; break;
            case 2: type = "Reserved"; break;
            case 3: type = "ACPI Reclaimable"; break;
            case 4: type = "NVS"; break;
            case 5: type = "Bad"; break;
            default: type = "Unknown"; break;
        }
        
        printf("base: %u, size: %u, type: %s\n", descriptor->base, descriptor->size, type);
    }

    for (;;) {
        asm("cli");
        asm("hlt");
    }
}