/**
 * E820
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/e820.h"

/**
 * Static Global Variables
*/

static ADDRESS_DESCRIPTOR descriptors[E820_MAX_ENTRIES * 2];
static E820_MAP smap;

/**
 * __add_region
*/

void __add_region(E820_ENTRY entry) {
    if (!entry.size) return;

    if (smap.index >= E820_MAX_ENTRIES) {
        printk("\033[33me820:\033[37m \033[91mtoo many entries\033[37m\n");
        return;
    }

    smap.entries[smap.index++] = entry;
}

/**
 * __insert_region
*/

void __insert_region(E820_ENTRY entry, uint32_t index) {
    if (!entry.size) return;

    if (smap.index >= E820_MAX_ENTRIES) {
        printk("\033[33me820:\033[37m \033[91mtoo many entries\033[37m\n");
        return;
    }

    for (uint32_t i = smap.index++; i > index; --i) smap.entries[i] = smap.entries[i - 1];

    smap.entries[index] = entry;
}

/**
 * __sanitize_e820
 * 
 * this is the best solution i've came up with
 * if you don't consider the sorting algorithm,
 * it resolves conflicts and merges regions in O(n)
 * 
 * it uses lowest and highest possible addresses of
 * address range descriptors and converts them to
 * points, these are sorted (l->h) and a new map is
 * constructed based on the new order
*/

void __sanitize_e820(uint32_t entries_count, E820_ENTRY *e820_entries) {
    printk("\033[33me820:\033[37m Sanitizing E820... ");
    
    e820_entries[entries_count++] = (E820_ENTRY) {
        .base = 0x00000000,
        .size = 1024+256+22*1024+768,
        .type = 2
    }; // IVT, BDA and stack

    e820_entries[entries_count++] = (E820_ENTRY) {
        .base = 0x0000d000,
        .size = 3*4096,
        .type = 2
    }; // page directory, 1. mib page table and kernel page table

    // TODO: reserve EBDA

    e820_entries[entries_count++] = (E820_ENTRY) {
        .base = 0x000a0000,
        .size = 128*1024+32*1024+160*1024+64*1024,
        .type = 2
    }; // video memory, video bios, bios expansion, rom bios

    e820_entries[entries_count++] = (E820_ENTRY) {
        .base = 0x00100000,
        .size = 4*1024*1024,
        .type = 2
    }; // kernel

    uint32_t descriptors_count = 0;

    for (uint32_t i = 0; i < entries_count; ++i) { // convert ards to points
        descriptors[descriptors_count].address = e820_entries[i].base;
        descriptors[descriptors_count].entry = &e820_entries[i];
        descriptors[descriptors_count++].end = FALSE;
        descriptors[descriptors_count].address = e820_entries[i].base + e820_entries[i].size - 1;
        descriptors[descriptors_count].entry = &e820_entries[i];
        descriptors[descriptors_count++].end = TRUE;
    }

    bool check;

    do { // sort descriptors
        check = FALSE;

        for (uint32_t i = 1; i < descriptors_count; ++i) {
            ADDRESS_DESCRIPTOR
                *previous = &descriptors[i - 1],
                *current = &descriptors[i];
            
            if (previous->address > current->address) {
                ADDRESS_DESCRIPTOR temp = *previous;
                *previous = *current;
                *current = temp;
                check = TRUE;
            } else if (previous->address == current->address) {
                if (previous->entry->type < current->entry->type) descriptors[i - 1] = descriptors[--descriptors_count];
                else descriptors[i] = descriptors[--descriptors_count];
                
                check = TRUE;
            }
        }
    } while (check);

    for (uint32_t i = 1; i < descriptors_count; ++i) { // takes care of merging regions (there is always an even number of descriptors)
        ADDRESS_DESCRIPTOR *previous_end = &descriptors[i - 1];
        ADDRESS_DESCRIPTOR *current_start = &descriptors[i];

        if (!current_start->end && previous_end->end && (previous_end->address + 1) == current_start->address && previous_end->entry->type == current_start->entry->type) previous_end->end = FALSE;
    }

    for (uint32_t i = 0; i < descriptors_count; ++i) { // create a new map
        ADDRESS_DESCRIPTOR *current = &descriptors[i];
        ADDRESS_DESCRIPTOR *found;

        if (current->end) {
            ADDRESS_DESCRIPTOR *previous = &descriptors[i - 1];

            __add_region((E820_ENTRY) {
                .base = previous->address + 1,
                .size = current->address - previous->address,
                .type = current->entry->type
            });
        } else {
            for (++i; i < descriptors_count; ++i) {
                found = &descriptors[i];

                if (found->entry->type > current->entry->type || ((found->entry->type == current->entry->type) && found->end)) break;
            }

            //if (!found) UNREACHABLE

            if (current->entry->type == found->entry->type) {
                __add_region((E820_ENTRY) {
                    .base = current->address,
                    .size = found->address - current->address + 1,
                    .type = current->entry->type
                });
            } else {
                __add_region((E820_ENTRY) {
                    .base = current->address,
                    .size = found->address - current->address,
                    .type = current->entry->type
                });

                --i;
            }
        }
    }

    smap.entries[2].base = 0x10001;
    printf("Ok\n");
}

char const *e820_get_type_string(E820_ENTRY *descriptor) {
    if (!descriptor) return NULL;

    switch (descriptor->type) {
        case 1: return "Free"; break; // usable memory, merge these regions if possible
        case 2: return "Reserved"; break; // don't use
        case 3: return "ACPI Reclaimable"; break; // we can use this after we're done with acpi
        case 4: return "ACPI NVS"; break; // don't use
        case 5: return "Bad"; break; // don't use
        default: return "Unknown"; break; // // don't use, change type to reserved?
    }
}

void __dump_e820(void) {
    E820_ENTRY *entries = smap.entries;
    uint32_t entries_count = smap.index;

    for (uint32_t i = 0; i < entries_count; ++i) {
        E820_ENTRY *descriptor = &entries[i];
        
        printk("\033[33me820:\033[37m %p - %p: %s\n", (void *)descriptor->base, descriptor->base + descriptor->size - 1, e820_get_type_string(descriptor));
    }
}

void *e820_rmalloc(uint32_t n, bool a) {
    if (!n) return NULL;

    uint32_t entries_count = smap.index;
    E820_ENTRY *entries = smap.entries;

    for (uint32_t i = 0; i < entries_count; ++i) {
        E820_ENTRY *entry = &entries[i];
        uint32_t
            base = entry->base,
            fixed_base = base,
            size = entry->size,
            diff = 0;

        if (a) {
            fixed_base = (fixed_base + 0x00000fff) & 0xfffff000;
            diff = fixed_base - base;
            size -= diff;
        }

        if (fixed_base + n > 1024*1024) break; // we're above first mib
        else if (entry->type == 1 && size >= n) {
            if (size > n) { // there is a remainder
                __insert_region((E820_ENTRY) {
                    .base = fixed_base + n,
                    .size = size - n,
                    .type = 1 // free
                }, i + 1);
            }

            entry->base = fixed_base;
            entry->size = n;
            entry->type = 2; // reserved

            if (diff) {
                __insert_region((E820_ENTRY) {
                    .base = base,
                    .size = diff,
                    .type = 1 // free
                }, i);
            }

            return fixed_base;
        }
    }

    return NULL;
}

E820_ENTRY *e820_get_descriptor(uint32_t index) {
    if (index >= smap.index) return NULL;

    return &smap.entries[index];
}