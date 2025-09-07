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
static ADDRESS_DESCRIPTOR *buffer[E820_MAX_ENTRIES * 2];
static uint32_t buffer_length = 0;
static E820_MAP smap;

/**
 * __add_region
*/

void __add_region(E820_ENTRY entry) {
    if (!entry.size)
        return;

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
 * this is the best solution i've came up with,
 * it sorts the address descriptors created from
 * entries and uses these to construct the new
 * memory map without overlapping areas
 * 
 * the algorithm takes the descriptor with the
 * highest type for the given range and pushes
 * the descriptor with lower type back to the
 * buffer for possible future use
 * 
 * i'll probably make this code look better
 * 
 * TODO:
 *  1) implement merging
 *  2) use buffer only (no current temp var)?
 *  3) get rid of the last operation outside
 *     the for cycle
*/

void __sanitize_e820(uint32_t count, E820_ENTRY *map) {
    map[count++] = (E820_ENTRY){
        .base = 0x00000000,
        .size = 1024+256+22*1024+768,
        .type = 2
    }; // IVT, BDA and stack

    map[count++] = (E820_ENTRY){
        .base = 0x0000d000,
        .size = 3*4096,
        .type = 2
    }; // page directory, 1st mib page table and kernel page table

    map[count++] = (E820_ENTRY){
        .base = 0x000a0000,
        .size = 128*1024+32*1024+160*1024+64*1024,
        .type = 2
    }; // video memory, video bios, bios expansion, rom bios

    map[count++] = (E820_ENTRY){
        .base = 0x00100000,
        .size = 4*1024*1024,
        .type = 2
    }; // kernel

    // convert map to list of address descriptors
    for (uint32_t i = 0; i < count; ++i) {
        E820_ENTRY *entry = &map[i];

        ADDRESS_DESCRIPTOR *start = &descriptors[i];
        start->address = entry->base;
        start->entry = entry;
    }

    bool swap;

    // let's use bubble sort here
    // TODO: use swap-index optimization?
    do {
        swap = FALSE;

        for (uint32_t i = 1; i < count; ++i) {
            // sort by address
            if (descriptors[i - 1].address > descriptors[i].address || (descriptors[i - 1].address == descriptors[i].address && descriptors[i - 1].entry->type > descriptors[i].entry->type)) {
                ADDRESS_DESCRIPTOR temp = descriptors[i - 1];
                descriptors[i - 1] = descriptors[i];
                descriptors[i] = temp;
                swap = TRUE;
            }
        }
    } while (swap);

    ADDRESS_DESCRIPTOR *current = &descriptors[0];
    uint32_t descriptor_index = 1;

    smap.index = 0;

    for (;;) {
        if (descriptor_index < count)
            buffer[buffer_length++] = &descriptors[descriptor_index++];

        ADDRESS_DESCRIPTOR *prioritized = NULL;
        uint32_t prioritized_index;

        if (!buffer_length)
            break;

        if (buffer_length) {
            prioritized = buffer[0];
            prioritized_index = 0;

            for (uint32_t i = 1; i < buffer_length; ++i) {
                ADDRESS_DESCRIPTOR *temp = buffer[i];

                if (temp->address < prioritized->address || (temp->address == prioritized->address && temp->entry->type > prioritized->entry->type)) {
                    prioritized_index = i;
                    prioritized = temp;
                }
            }
        }

        uint32_t base;
        uint32_t size;
        uint32_t type;

        if ((current->entry->base + current->entry->size - 1) >= prioritized->address) {
            // overlap
            ADDRESS_DESCRIPTOR *to_push;

            if (current->entry->type <= prioritized->entry->type) {
                base = current->address;
                size = prioritized->address - current->address;
                type = current->entry->type;

                uint32_t prev_address = current->address;
                current->address = prioritized->entry->base + prioritized->entry->size;

                if (current->address < prev_address || current->address >= (current->entry->base + current->entry->size - 1)) {
                    buffer[prioritized_index] = buffer[--buffer_length];
                } else
                    buffer[prioritized_index] = current;

                current = prioritized;
            } else { // current->entry->type > prioritized->entry->type
                uint32_t prev_address = prioritized->address;
                prioritized->address = current->entry->base + current->entry->size;

                if (prioritized->address < prev_address || prioritized->address >= (prioritized->entry->base + prioritized->entry->size - 1))
                    buffer[prioritized_index] = buffer[--buffer_length];
            }
        } else {
            // no overlap
            base = current->address;
            size = current->entry->size - (current->address - current->entry->base);
            type = current->entry->type;

            current = prioritized;
            buffer[prioritized_index] = buffer[--buffer_length];
        }

        __add_region((E820_ENTRY){
            .base = base,
            .size = size,
            .type = type
        });
    }

    // last descriptor
    __add_region((E820_ENTRY){
        .base = current->address,
        .size = current->entry->size - (current->address - current->entry->base),
        .type = current->entry->type
    });
}

/**
 * __get_system_memory
*/

E820_ENTRY *__get_last_entry(void) {
    return &smap.entries[smap.index - 1];
}

/**
 * e820_get_type_string
*/

char const *e820_get_type_string(E820_ENTRY *descriptor) {
    if (!descriptor) return NULL;

    switch (descriptor->type) {
        case REGION_FREE:
            return "Free"; // usable memory, merge these regions if possible
            break;
        
        case REGION_RESERVED:
            return "Reserved"; // don't use
            break;
        
        case REGION_ACPI_RECLAIMABLE:
            return "ACPI Reclaimable"; // we can use this after we're done with acpi
            break;
        
        case REGION_ACPI_NVS:
            return "ACPI NVS"; // don't use
            break;
        
        case REGION_BAD:
            return "Bad"; // don't use
            break;
        
        default:
            return "Unknown"; // don't use, change type to reserved?
            break;
    }
}

/**
 * __dump_e820
*/

void __dump_e820(void) {
    E820_ENTRY *entries = smap.entries;
    uint32_t entries_count = smap.index;

    for (uint32_t i = 0; i < entries_count; ++i) {
        E820_ENTRY *descriptor = &entries[i];
        
        printk("\033[33me820:\033[37m %p - %p: %s\n", (void *)descriptor->base, descriptor->base + descriptor->size - 1, e820_get_type_string(descriptor));
    }
}

/**
 * e820_amalloc
*/

void *e820_amalloc(uint32_t n, bool a) {
    if (!n)
        return NULL;

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

        if (entry->type == 1 && size >= n) {
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

            return (void *)fixed_base;
        }
    }

    return NULL;
}

/**
 * e820_malloc
*/

void *e820_malloc(uint32_t n) {
    return e820_amalloc(n, FALSE);
}

// TODO: merge with e820_amalloc!!!
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

            return (void *)fixed_base;
        }
    }

    return NULL;
}

E820_ENTRY *e820_get_descriptor(uint32_t index) {
    if (index >= smap.index) return NULL;

    return &smap.entries[index];
}