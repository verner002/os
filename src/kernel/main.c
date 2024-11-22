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

/**
 * Main
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

    for (;;) {
        asm("cli");
        asm("hlt");
    }
}