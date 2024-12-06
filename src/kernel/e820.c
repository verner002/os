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

static E820_MAP sanitized_e820;

/**
 * __add_region
*/

static void __add_region(E820_MAP *map, E820_ENTRY entry) {
    if (!entry.size) return;

    if (map->index >= E820_MAX_ENTRIES) {
        printk("\033[33me820:\033[37m \033[91mtoo many regions in map\033[37m\n");
        return;
    }

    map->entries[map->index++] = entry;
}

/**
 * __sanitize_e820
*/

E820_MAP *__sanitize_e820(unsigned int entries_count, E820_ENTRY *map) {
    bool check;

    do { // sort entries
        check = FALSE;

        for (unsigned int i = 1; i < entries_count; ++i) {
            E820_ENTRY
                *previous_entry = &map[i - 1],
                *current_entry = &map[i];
            
            if (previous_entry->base > current_entry->base || (previous_entry->base == current_entry->base && previous_entry->type > current_entry->type)) {
                E820_ENTRY temp = map[i - 1];
                map[i - 1] = map[i];
                map[i] = temp;
                check = TRUE;
            }
        }
    } while (check);

    E820_MAP *_map = &sanitized_e820;

    for (unsigned int i = 0; i < entries_count; ++i) {
        E820_ENTRY *current_entry = &map[i];

        /*if (i + 1 < entries_count) { // we have another entry to work with
            E820_ENTRY *next_entry = &map[i + 1];

            if (current_entry->base == next_entry->base) {
                if (next_entry->size > current_entry->size) {
                    __add_region(_map, *current_entry);
                    
                    E820_ENTRY tmp = *next_entry; // FIXME: change next_entry
                    tmp.base = current_entry->base + current_entry->size;
                    tmp.size = tmp.size - tmp.base;

                    if (i + 2 < entries_count) { // + sorting condition
                        // swap
                        --i;
                    }
                }
            }
        } else*/ __add_region(&sanitized_e820, *current_entry);
    }

    return _map;
}

void dump_e820(unsigned int entries_count, E820_ENTRY *map) {
    for (unsigned int i = 0; i < entries_count; ++i) {
        E820_ENTRY *descriptor = &map[i];

        char *type;

        switch (descriptor->type) {
            case 1: type = "Free"; break; // usable memory, merge these regions if possible
            case 2: type = "Reserved"; break; // don't use
            case 3: type = "ACPI Reclaimable"; break; // we can use this after we're done with acpi
            case 4: type = "ACPI NVS"; break; // don't use
            case 5: type = "Bad"; break; // don't use
            default: type = "Unknown"; break; // // don't use, change type to reserved?
        }
        
        printk("\033[33me820:\033[37m %p - %p: %s\n", (void *)descriptor->base, descriptor->base + descriptor->size, type);
    }
}

void *e820_alloc(unsigned int n) {
    return NULL;
}