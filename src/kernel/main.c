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

void entry(unsigned int smap_entries_count, SMAP_ENTRY *smap) {
    printf("Kernel ready!\n");

    for (unsigned int i = 0; i < smap_entries_count; ++i) {
        SMAP_ENTRY *entry = &smap[i];
        
        printf("base: %u, size: %u, type: %u\n", entry->base, entry->size, entry->type);
    }

    for (;;) {
        asm("cli");
        asm("hlt");
    }
}