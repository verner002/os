/**
 * @file e820.c
 * @author verner002
 * @date 23/09/2025
*/

#include <kernel/e820.h>
#include <kernel/printk.h>

typedef struct address_descriptor ADDRESS_DESCRIPTOR;

struct address_descriptor {
    uint32_t address;
    struct e820_entry *entry;
};

extern struct e820_map mmap;

static ADDRESS_DESCRIPTOR descriptors[E820_MAX_ENTRIES * 2];
static ADDRESS_DESCRIPTOR *buffer[E820_MAX_ENTRIES * 2];
static uint32_t buffer_length = 0;

/**
 * add_region
*/

static void add_region(struct e820_entry entry) {
    if (!entry.size)
        return;

    if (mmap.nentries >= E820_MAX_ENTRIES) {
        printk("\033[33me820:\033[37m \033[91mtoo many entries\033[37m\n");
        return;
    }

    mmap.entries[mmap.nentries++] = entry;
}

/**
 * insert_region
*/

static void insert_region(struct e820_entry entry, size_t index) {
    if (!entry.size)
        return;

    if (mmap.nentries >= E820_MAX_ENTRIES) {
        printk("\033[33me820:\033[37m \033[91mtoo many entries\033[37m\n");
        return;
    }

    for (size_t i = mmap.nentries++; i > index; --i)
        mmap.entries[i] = mmap.entries[i - 1];

    mmap.entries[index] = entry;
}

/**
 * e820_align_mmap
 * 
 * FIXME: we have to maximize reserved areas if needed and
 *  minimize free areas (that ensures corrent page alignment)
*/

/*static void e820_align_mmap(void) {
    uint32_t count = mmap.count;

    uint32_t new_index = 0;

    for (uint32_t i = 0; i < count; ++i) {
        struct e820_entry *entry = &mmap.entries[i];
        
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

        mmap.entries[new_index++] = (struct e820_entry){
            .base = aligned_base,
            .size = aligned_size,
            .type = type
        };
    }

    mmap.count = new_index;
}*/

/**
 * e820_sanitize
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
 *  4) implement page aligning
*/

void e820_sanitize(size_t e820_nentries, struct e820_entry *e820_entries) {
    /*e820_entries[e820_nentries++] = (struct e820_entry){
        .base = 0x00000000,
        .size = 512*1024,
        .type = 2
    }; // kernel

    e820_entries[e820_nentries++] = (struct e820_entry){
        .base = 0x00080000,
        .size = 64*1024,
        .type = 2
    }; // page directory, page table, system information block, kernel stack

    e820_entries[e820_nentries++] = (struct e820_entry){
        .base = 0x000a0000,
        .size = 128*1024+32*1024+160*1024+64*1024,
        .type = 2
    };*/ // video memory, video bios, bios expansion, rom bios

    // kernel and kernel stack
    e820_entries[e820_nentries++] = (struct e820_entry){
        .base = 0x00000000,
        .size = 0x00080000,
        .type = 2
    };

    e820_entries[e820_nentries++] = (struct e820_entry){
        .base = 0x00080000,
        .size = /*2*4096*/ 0x10000, // FIXME: temp solution
        .type = 2
    }; // pd and pt

    // video memory, video bios, bios expansion and rom bios
    e820_entries[e820_nentries++] = (struct e820_entry){
        .base = 0x000a0000,
        .size = 128*1024+32*1024+160*1024+64*1024,
        .type = 2
    };

    // convert map to list of address descriptors
    for (size_t i = 0; i < e820_nentries; ++i) {
        struct e820_entry *entry = &e820_entries[i];

        ADDRESS_DESCRIPTOR *start = &descriptors[i];
        start->address = entry->base;
        start->entry = entry;
    }

    int swap;

    // let's use bubble sort here
    // TODO: use swap-index optimization?
    do {
        swap = 0;

        for (size_t i = 1; i < e820_nentries; ++i) {
            // sort by address
            if (descriptors[i - 1].address > descriptors[i].address || (descriptors[i - 1].address == descriptors[i].address && descriptors[i - 1].entry->type > descriptors[i].entry->type)) {
                ADDRESS_DESCRIPTOR temp = descriptors[i - 1];
                descriptors[i - 1] = descriptors[i];
                descriptors[i] = temp;
                swap = 1;
            }
        }
    } while (swap);

    ADDRESS_DESCRIPTOR *current = &descriptors[0];
    uint32_t descriptor_index = 1;

    mmap.nentries = 0;

    for (;;) {
        if (descriptor_index < e820_nentries)
            buffer[buffer_length++] = &descriptors[descriptor_index++];

        ADDRESS_DESCRIPTOR *prioritized = NULL;
        uint32_t prioritized_index;

        if (!buffer_length)
            break;

        prioritized = buffer[0];
        prioritized_index = 0;

        for (size_t i = 1; i < buffer_length; ++i) {
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

        add_region((struct e820_entry){
            .base = base,
            .size = size,
            .type = type
        });
    }

    // last descriptor
    add_region((struct e820_entry){
        .base = current->address,
        .size = current->entry->size - (current->address - current->entry->base),
        .type = current->entry->type
    });

    // TODO: implement the function here
    //e820_align_mmap();
}

/**
 * e820_dump_mmap
*/

void e820_dump_mmap(void) {
    static char *types[] = {
        "Unknown",
        "Free",
        "Reserved",
        "ACPI Reclaimable",
        "ACPI NVS",
        "Bad"
    };

    struct e820_entry *entries = mmap.entries;
    struct e820_entry *entries_end = entries + mmap.nentries;

    for (; entries < entries_end; ++entries) {
        uint32_t base = entries->base;
        uint32_t type = entries->type;

        if (type >= (sizeof(types) / sizeof(*types)))
            type = 0;
        
        printk("\033[33me820:\033[37m %p - %p: %s\n",
            base,
            base + entries->size - 1,
            types[type]
        );
    }
}

/**
 * e820_alloc
 * 
 * @arg n Number of bytes to allocate
 * @arg a Return page-aligned address
 * @arg l Address space limit (0 for no limit)
*/

void *e820_alloc(uint32_t n, int a, uint32_t l) {
    if (!n)
        return NULL;

    size_t nentries = mmap.nentries;
    struct e820_entry *entries = mmap.entries;

    for (size_t i = 0; i < nentries; ++i) {
        struct e820_entry *entry = &entries[i];
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

        if (l && fixed_base + n > l)
            break;
        else if (entry->type == 1 && size >= n) {
            if (size > n) { // there is a remainder
                insert_region((struct e820_entry) {
                    .base = fixed_base + n,
                    .size = size - n,
                    .type = 1 // free
                }, i + 1);
            }

            entry->base = fixed_base;
            entry->size = n;
            entry->type = 2; // reserved

            if (diff) {
                insert_region((struct e820_entry) {
                    .base = base,
                    .size = diff,
                    .type = 1 // free
                }, i);
            }

            /*printk("\033[33me820:\033[37m update: %p - %p: %s\n",
                fixed_base,
                fixed_base + n - 1,
                "Reserved"
            );*/

            return (void *)fixed_base;
        }
    }

    return NULL;
}

/**
 * e820_malloc
*/

void *e820_malloc(uint32_t n) {
    return e820_alloc(n, 0, 0);
}

/**
 * e820_rmalloc
*/

void *e820_rmalloc(uint32_t n, int a) {
    return e820_alloc(n, a, 4*1024*1024);
}