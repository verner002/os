/**
 * @file e820.c
 * @author verner002
 * @date 23/09/2025
*/

#include "null.h"
#include "macros.h"
#include "mm/e820.h"
#include "kstdlib/stdio.h"

typedef struct __address_descriptor ADDRESS_DESCRIPTOR;

struct __address_descriptor {
    uint32_t address;
    E820_ENTRY *entry;
};

static ADDRESS_DESCRIPTOR descriptors[E820_MAX_ENTRIES * 2];
static ADDRESS_DESCRIPTOR *buffer[E820_MAX_ENTRIES * 2];
static uint32_t buffer_length = 0;
E820_MAP smap;

/**
 * __add_region
*/

static void __add_region(E820_ENTRY entry) {
    if (!entry.size)
        return;

    if (smap.count >= E820_MAX_ENTRIES) {
        printk("\033[33me820:\033[37m \033[91mtoo many entries\033[37m\n");
        return;
    }

    smap.entries[smap.count++] = entry;
}

/**
 * __insert_region
*/

static void __insert_region(E820_ENTRY entry, uint32_t index) {
    if (!entry.size)
        return;

    if (smap.count >= E820_MAX_ENTRIES) {
        printk("\033[33me820:\033[37m \033[91mtoo many entries\033[37m\n");
        return;
    }

    for (uint32_t i = smap.count++; i > index; --i)
        smap.entries[i] = smap.entries[i - 1];

    smap.entries[index] = entry;
}

/**
 * __e820_align_mmap
 * 
 * FIXME: we have to maximize reserved areas if needed and
 *  minimize free areas (that ensures corrent page alignment)
*/

/*static void __e820_align_mmap(void) {
    uint32_t count = smap.count;

    uint32_t new_index = 0;

    for (uint32_t i = 0; i < count; ++i) {
        E820_ENTRY *entry = &smap.entries[i];
        
        uint32_t
            aligned_base,
            aligned_size,
            type = entry->type;
        
        if (type == REGION_FREE) {
            aligned_base = (entry->base + 0x00000fff) & 0xfffff000;
            uint32_t base_diff = aligned_base - entry->base;
            aligned_size = entry->size;

            if (base_diff >= aligned_size)
                continue;

            aligned_size = (aligned_size - base_diff) & 0xfffff000;

            if (!aligned_size)
                continue;
        } else {
            aligned_base = entry->base & 0xfffff000;
            uint32_t base_diff = entry->base - aligned_base;
            aligned_size = (aligned_size + base_diff + 0x00000fff) & 0xfffff000;
        }

        smap.entries[new_index++] = (E820_ENTRY){
            .base = aligned_base,
            .size = aligned_size,
            .type = type
        };
    }

    smap.count = new_index;
}*/

/**
 * __e820_sanitize
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

void __e820_sanitize(uint32_t count, E820_ENTRY *map) {
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

    smap.count = 0;

    for (;;) {
        if (descriptor_index < count)
            buffer[buffer_length++] = &descriptors[descriptor_index++];

        ADDRESS_DESCRIPTOR *prioritized = NULL;
        uint32_t prioritized_index;

        if (!buffer_length)
            break;

        prioritized = buffer[0];
        prioritized_index = 0;

        for (uint32_t i = 1; i < buffer_length; ++i) {
            ADDRESS_DESCRIPTOR *temp = buffer[i];

            if (temp->address < prioritized->address || (temp->address == prioritized->address && temp->entry->type > prioritized->entry->type)) {
                prioritized_index = i;
                prioritized = temp;
            }
        }

        uint32_t base;
        uint32_t size;
        uint32_t type;

        if ((current->entry->base + current->entry->size - 1) >= prioritized->address) {
            // overlap
            // TODO: this if-else-block could be merged into one block of code
            if (current->entry->type <= prioritized->entry->type) {
                base = current->address;
                size = prioritized->address - current->address;
                type = current->entry->type;

                uint32_t prev_address = current->address;
                current->address = prioritized->entry->base + prioritized->entry->size;

                if (current->address < prev_address || current->address >= (current->entry->base + current->entry->size - 1))
                    buffer[prioritized_index] = buffer[--buffer_length];
                else
                    buffer[prioritized_index] = current;

                current = prioritized;
            } else { // current->entry->type > prioritized->entry->type
                base = prioritized->address;
                size = current->address - prioritized->address;
                type = prioritized->entry->type;

                uint32_t prev_address = prioritized->address;
                prioritized->address = current->entry->base + current->entry->size;

                if (prioritized->address < prev_address || prioritized->address >= (prioritized->entry->base + prioritized->entry->size - 1))
                    buffer[prioritized_index] = buffer[--buffer_length];
                /*else
                    buffer[prioritized_index] = prioritized;*/
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

    // TODO: implement the function here
    //__e820_align_mmap();
}

/**
 * __e820_dump_mmap
*/

void __e820_dump_mmap(void) {
    static char *types[] = {
        "Unknown",
        "Free",
        "Reserved",
        "ACPI Reclaimable",
        "ACPI NVS",
        "Bad"
    };

    E820_ENTRY *entries = smap.entries;
    uint32_t entries_count = smap.count;

    for (uint32_t i = 0; i < entries_count; ++i) {
        E820_ENTRY *descriptor = &entries[i];

        uint32_t base = descriptor->base;
        uint32_t type = descriptor->type;

        if (type >= sizeofarray(types))
            type = 0;
        
        printk("\033[33me820:\033[37m %p - %p: %s\n",
            base,
            base + descriptor->size - 1,
            types[type]
        );
    }
}

/**
 * __e820_alloc
*/

void *__e820_alloc(uint32_t n, bool a, bool f) {
    if (!n)
        return NULL;

    uint32_t entries_count = smap.count;
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
            size = diff < size ? (size - diff) : 0;
        }

        if (f && fixed_base + n > 1024*1024)
            break;
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

/**
 * __e820_malloc
*/

void *__e820_malloc(uint32_t n) {
    return __e820_alloc(n, FALSE, FALSE);
}

/**
 * __e820_rmalloc
*/

void *__e820_rmalloc(uint32_t n, bool a) {
    return __e820_alloc(n, a, TRUE);
}