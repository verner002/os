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

static void __add_region(E820_ENTRY entry) {
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

static void __insert_region(E820_ENTRY entry, uint32_t index) {
    if (!entry.size) return;

    if (smap.index >= E820_MAX_ENTRIES) {
        printk("\033[33me820:\033[37m \033[91mtoo many entries\033[37m\n");
        return;
    }

    for (uint32_t i = smap.index++; i > index; --i) smap.entries[i] = smap.entries[i - 1];

    smap.entries[index] = entry;
}

/**
 * __init_e820
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

void __init_e820(uint32_t entries_count, E820_ENTRY *e820_entries) {
    if (entries_count == 1) {
        __add_region(e820_entries[0]);
        return;
    }

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
            }
        }
    } while (check);

    for (uint32_t i = 2; i < descriptors_count; i += 2) { // takes care of merging regions (there is always an even number of descriptors)
        ADDRESS_DESCRIPTOR *previous_end = &descriptors[i - 1];
        ADDRESS_DESCRIPTOR *current_start = &descriptors[i];

        if ((previous_end->address + 1) == current_start->address && previous_end->entry->type == current_start->entry->type) previous_end->end = FALSE;
    }

    for (uint32_t i = 0; i < descriptors_count; ++i) { // create a new map
        ADDRESS_DESCRIPTOR *current = &descriptors[i];
        ADDRESS_DESCRIPTOR *found;

        if (current->end) {
            ADDRESS_DESCRIPTOR *previous = &descriptors[i - 1];

            __add_region((E820_ENTRY) {
                .base = previous->address + 1,
                .size = current->address - previous->address + 1,
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

void dump_e820(void) {
    E820_ENTRY *entries = smap.entries;
    uint32_t entries_count = smap.index;

    for (uint32_t i = 0; i < entries_count; ++i) {
        E820_ENTRY *descriptor = &entries[i];
        
        printk("\033[33me820:\033[37m %p - %p: %s\n", (void *)descriptor->base, descriptor->base + descriptor->size - 1, e820_get_type_string(descriptor));
    }
}

void *e820_alloc(uint32_t n) {
    if (!n) return NULL;

    uint32_t entries_count = smap.index;
    E820_ENTRY *entries = smap.entries;
    uint32_t size = n * 4096;
    uint32_t address = 0;

    for (uint32_t i = 0; i < entries_count; ++i) {
        E820_ENTRY *entry = &entries[i];

        if (entry->type == 1 && entry->size >= size) {
            E820_ENTRY remainder = {
                .base = entry->base + size,
                .size = entry->size - size,
                .type = 1 // free
            };

            //printk("\033[33me820:\033[37m updating %p, %u, %s\n", remainder.base, remainder.size, e820_get_type_string(&remainder));

            address = entry->base;
            entry->size = size;
            entry->type = 2; // reserved

            __insert_region(remainder, i + 1);
            break;
        }
    }

    return (void *)address;
}

E820_ENTRY *e820_get_descriptor(uint32_t index) {
    if (index >= smap.index) return NULL;

    return &smap.entries[index];
}