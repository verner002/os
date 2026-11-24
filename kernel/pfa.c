/**
 * @file pfa.c
 * @author verner002
 * @date 23/09/2025
*/

#include <stddef.h>

#include <kernel/pfa.h>
#include <kernel/system.h>
#include <kernel/paging.h>

// TODO: split memory to LOWMEM and HIGHMEM
// use LOWMEM for kernel allocations and
// HIGHMEM for userland and LOWMEM only if
// there is no HIGHMEM left
static uint32_t *bitmap;
static size_t size;
static size_t last_index;

#include <kernel/printk.h>

static inline void pfa_free_pfn(uint32_t pfn) {
    uint32_t idx = pfn >> 5;

    bitmap[idx] &= ~(1u << (pfn & 31));

    if (idx < last_index)
        last_index = idx;
}

int pfa_init(struct e820_map *mmap) {
    // we use the last entry to calculate
    // the physical memory bitmap size
    struct e820_entry *entries = mmap->entries;
    size_t nentries = mmap->nentries;

    struct e820_entry *last_entry = NULL;

    for (size_t i = nentries; i > 0; --i) {
        struct e820_entry *entry = entries + i - 1;

        if (entry->type == 1) {
            last_entry = entry;
            break;
        }
    }

    if (!last_entry)
        return -1;

    size_t const pages_count = (size_t)((last_entry->base / PAGE_SIZE) + (last_entry->size / PAGE_SIZE));

    size_t const bitmap_size = ((pages_count + 31) / 32) * sizeof(uint32_t);

    printk("bitmap size = %u\n", bitmap_size);

    // memory for pager bitmap
    uint32_t bitmap_phys = (uint32_t)e820_alloc(bitmap_size, 1, 768 * (1 << 20));

    if (!bitmap_phys)
        return -1;

    bitmap = (uint32_t *)(bitmap_phys + KERNEL_VIRT_BASE);

    size = (pages_count + 31) / 32;

    // set all pages as reserved
    for (size_t i = 0; i < size; ++i)
        bitmap[i] = 0xffffffff; // reserved

    // free only regions with type "usable"
    // in e820 record
    for (size_t i = 0; i < nentries; ++i) {
        struct e820_entry *entry = &(mmap->entries[i]);

        if (entry->type != 1)
            continue;

        // FIXME: what if base is not aligned?
        size_t pages = (size_t)(entry->size / PAGE_SIZE);

        if (pages < 1)
            continue;

        uint32_t base = entry->base;
        uint32_t end = base + entry->size - 1;

        uint32_t first = (base >> 12) + ((base & 0xfffu) != 0);
        uint32_t last = (end >> 12) + ((end & 0xfffu) == 0xfffu);

        if (first >= last)
            continue;
        
        for (uint32_t pfn = first; pfn < last; ++pfn)
            pfa_free_pfn(pfn);
    }

    // reset last index
    last_index = 0;
    return 0;
}

void pfa_reserve(void *ptr) {
    size_t i = (size_t)ptr / PAGE_SIZE;
    size_t idx = i / 32;

    bitmap[idx] |= 1u << (i % 32);
}

void *pfa_alloc(void) {
    for (size_t i = last_index; i < size; ++i) {
        if (~bitmap[i]) {
            uint32_t chunk = bitmap[i];
            uint32_t index = chunk;
            chunk |= chunk + 1;
            bitmap[i] = chunk;
            index ^= chunk; // toggles the first 0 bit it finds
            
            // TODO: fact-check this (use bsf?)
            // fast log2 is faster on
            // older hardware (bsr could
            // possibly take up to about
            // 105 cpu cycles) 
            register uint32_t page =
                (((index & 0xaaaaaaaa) != 0) << 0) |
                (((index & 0xcccccccc) != 0) << 1) |
                (((index & 0xf0f0f0f0) != 0) << 2) |
                (((index & 0xff00ff00) != 0) << 3) |
                (((index & 0xffff0000) != 0) << 4);

            /*asm volatile (
                "bsr %0, %1"
                : "=r" (page)
                : "r" (index)
                :
            ); // returns toggled bit index*/

            last_index = i;

            return (void *)(((i << 5) + page) * PAGE_SIZE);
        }
    }

    return NULL;
}

void pfa_dealloc(void *ptr) {
    pfa_free_pfn((uint32_t)ptr >> 12);
}

void *blkalloc(int n) {
    if (!n) return NULL;

    uint32_t chunks_count = n / 16; // number of words
    uint32_t pages_count = n % 16; // number of bits

    if (chunks_count) { // handles allocation of large blocks of memory
        uint32_t last_chunk = size - chunks_count + 1;
        uint32_t chunks_to_find = chunks_count;
        uint32_t i = 0; // TODO: start from offset

        for (; i < last_chunk; ++i) {
            if (!bitmap[i]) {
                uint32_t j = i;

                do {
                    if (bitmap[j]) break;
                    else if (!--chunks_to_find) break;
                } while (++j < last_chunk);
                // bitmap[j] is always checked after the while loop

                if (!chunks_to_find) {
                    if (!pages_count) {
                        // TODO: allocate, change offset
                        return (void *)(i * 16 * PAGE_SIZE); // ### THIS ###
                    } // else...

                    // mov ecx, dword [pages_count]
                    // dec ecx
                    // mov eax, 0x00008000
                    // sar eax, cl
                    if (i > 0 && !(bitmap[i - 1] & ~(((uint16_t)0x8000 >> (pages_count - 1)) - 1))) {
                        // TODO: allocate, change offset
                        return (void *)((i * 16 - pages_count) * PAGE_SIZE);
                    }
                    
                    // mov ecx, dword [pages_count]
                    // mov eax, 0x0000001
                    // shl eax, cl
                    // dec eax
                    uint32_t l = ++j/* + 1*/; // ### HERE ###
                    if (l < size && !(bitmap[l] & (((uint16_t)0x0001 << pages_count) - 1))) {
                        // TODO: allocate, change offset
                        return (void *)(i * 16 * PAGE_SIZE); // TODO: merge with ### THIS ###?
                    }

                    // ++j? (bitmap[j + 1] cannot be 0), place ### HERE ###?
                }
                
                chunks_to_find = chunks_count;
                i = j;
            }

            /*if (bitmap[i]) chunks_to_find = chunks_count;
            else if (!--chunks_to_find) { ... };*/
        }
    } else { // handles allocation of small blocks of memory (up to 15 pages)
        for (uint32_t i = 0; i < size - 1; ++i) {
            if (~bitmap[i]) {
                uint32_t dchunk = ((uint32_t)bitmap[i + 1] << 16) | bitmap[i];
                uint32_t mask = ((uint32_t)0x00000001 << pages_count) - 1;

                for (uint32_t j = 0; j < 32 - pages_count; ++j) {
                    if (!(dchunk & (mask << j))) {
                        // TODO: change offset
                        bitmap[i] |= (uint16_t)(mask << j);
                        bitmap[i + 1] |= (uint16_t)((mask << j) >> 16);
                        return (void *)((i * 16 + j) * PAGE_SIZE);
                    }
                }
            }
        }
    }

    return NULL; // out of memory, change offset(?)
}