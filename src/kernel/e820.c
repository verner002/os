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

static void __insert_region(E820_ENTRY entry, unsigned int index) {
    if (!entry.size) return;

    if (smap.index >= E820_MAX_ENTRIES) {
        printk("\033[33me820:\033[37m \033[91mtoo many entries\033[37m\n");
        return;
    }

    for (unsigned int i = smap.index++; i > index; --i) smap.entries[i] = smap.entries[i - 1];

    smap.entries[index] = entry;
}

/**
 * __init_e820
*/

void __init_e820(unsigned int entries_count, E820_ENTRY *e820_entries) {
    bool check;

    do { // sort entries, TODO: insert descriptors with missing ranges?
        check = FALSE;

        for (unsigned int i = 1; i < entries_count; ++i) {
            E820_ENTRY
                *previous_entry = &e820_entries[i - 1],
                *current_entry = &e820_entries[i];
            
            if (previous_entry->base > current_entry->base || (previous_entry->base == current_entry->base && previous_entry->type > current_entry->type)) {
                E820_ENTRY temp = e820_entries[i - 1];
                e820_entries[i - 1] = e820_entries[i];
                e820_entries[i] = temp;
                check = TRUE;
            }
        }
    } while (check);

    for (unsigned int i = 0; i < entries_count; ++i) {
        E820_ENTRY *current_entry = &e820_entries[i];

        if (current_entry->type == 1) {
            unsigned int divident = current_entry->base & 0xfffff000;
            unsigned int remainder = current_entry->base & 0x00000fff;

            if (divident < 0xfffff000 || !remainder) {
                unsigned int new_size = current_entry->size - remainder;

                if (current_entry->size > remainder && new_size > 0x00001000) {
                    // page align base
                    if (remainder) current_entry->base = divident + 0x00001000;

                    // page align size
                    current_entry->size = new_size & 0xfffff000;
                } else continue; // don't add
            } else continue; // don't add
        }

        /*if (i + 1 < entries_count) { // we have another entry to work with
            E820_ENTRY *next_entry = &e820_entries[i + 1];

            if (current_entry->base == next_entry->base) {
                if (next_entry->size >= current_entry->size) {
                    next_entry->base = current_entry->base + current_entry->size;
                    next_entry->size -= current_entry->size;

                    if (!next_entry->size) ++i; // TODO: swap?
                    else if (i + 2 < entries_count) {
                        E820_ENTRY *next_next_entry = &e820_entries[i + 2];

                        if (next_entry->base > next_next_entry->base || (next_entry->base == next_next_entry->base && next_entry->type > next_next_entry->type)) {
                            E820_ENTRY temp = *next_next_entry;
                            e820_entries[i + 2] = *next_entry;
                            e820_entries[i + 1] = temp;
                        }
                    }
                }
            } else { // current_entry->base < next_entry->base

            }
        } else*/ __add_region(*current_entry);
    }

    // TODO: reserve memory for IVT and BDA if not already reserved
}

char *e820_get_type_string(E820_ENTRY *descriptor) {
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
    unsigned int entries_count = smap.index;

    for (unsigned int i = 0; i < entries_count; ++i) {
        E820_ENTRY *descriptor = &entries[i];
        
        printk("\033[33me820:\033[37m %p - %p: %s\n", (void *)descriptor->base, descriptor->base + descriptor->size - 1, e820_get_type_string(descriptor));
    }
}

void *e820_alloc(unsigned int n) {
    if (!n) return NULL;

    unsigned int entries_count = smap.index;
    E820_ENTRY *entries = smap.entries;
    unsigned int size = n * 4096;
    unsigned int address = 0;

    for (unsigned int i = 0; i < entries_count; ++i) {
        E820_ENTRY *entry = &entries[i];

        if (entry->type == 1 && entry->size >= size) {
            E820_ENTRY remainder = {
                .base = entry->base + size,
                .size = entry->size - size,
                .type = 1 // free
            };

            printk("\033[33me820:\033[37m updating %p, %u, %s\n", remainder.base, remainder.size, e820_get_type_string(&remainder));

            address = entry->base;
            entry->size = size;
            entry->type = 2; // reserved

            __insert_region(remainder, i + 1);
            break;
        }
    }

    return (void *)address;
}

E820_ENTRY *e820_get_descriptor(unsigned int index) {
    if (index >= smap.index) return NULL;

    return &smap.entries[index];
}