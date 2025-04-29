/**
 * Physical Memory Manager (Page Frame Allocator, Pager)
 * 
 * Author: verner002
 * 
 * NOTE:
 *  let's use a bitmap allocator for a while
 *  and move to an avl-tree base later
*/

/**
 * Includes
*/

#include "kernel/pager.h"

/**
 * Static Global Variables
*/

static uint32_t *bitmap; // size=x/(4096*8) to represent x bytes
static uint32_t size; // number of uint32_t in bitmap
static uint32_t last_index;

/**
 * init_pager
*/

int32_t __init_pager(void) {
    printk("\033[33mpmm:\033[37m Initializing... ");

    E820_ENTRY *last_entry = __get_last_entry();

    uint32_t pages_count = last_entry->base / 4096 + last_entry->size / 4096;

    bitmap = (uint32_t *)e820_rmalloc(pages_count / 8, TRUE);

    if (!bitmap)
        return -1;

    size = pages_count / 32;

    // initialize bitmap
    bitmap[0] = 0x00000001;

    for (uint32_t i = 1; i < size; ++i)
        bitmap[i] = 0; // free

    // reserve memory used by bitmap
    for (uint32_t i = 0; i < pages_count; ++i)
        pgreserve(bitmap + i * 4096);

    // reset last index
    last_index = 0;

    printf("Ok\n");
    return 0;
}

/**
 * pgreserve
*/

void pgreserve(void *p) {
    if (!p) return;

    uint32_t i = (uint32_t)p / 4096;

    bitmap[i / 32] |= 1 << (i % 32);

    //if (i < last_index) last_index = ++i; // update offset
}

/**
 * pgalloc
 * 
 * Note:
 *  Allocates a page.
*/

void *pgalloc(void) {
    for (uint32_t i = last_index; i < size; ++i) {
        if (~bitmap[i]) {
            uint32_t temp = bitmap[i];
            temp ^= bitmap[i] |= temp + 1; // toggles the first 0 bit it finds

            asm (
                "push eax\t\n"
                "bsr eax, %1\t\n"
                "mov %0, eax\t\n"
                "pop eax"
                : "=m" (temp)
                : "m" (temp)
                :
            ); // returns toggled bit index
            
            // fast log2
            /*register uint32_t r = (temp & 0x0000aaaa) != 0;
            r |= ((temp & 0x0000cccc) != 0) << 1;
            r |= ((temp & 0x0000f0f0) != 0) << 2;
            r |= ((temp & 0x0000ff00) != 0) << 3;*/

            last_index = i; // update offset
            
            return (void *)((i * 32 + (uint32_t)temp) * 4096);
        }
    }

    return NULL;
}

/**
 * pgfree
 * 
 * Note:
 *  Deallocates a page.
*/

void pgfree(void *p) {
    if (!p) return;

    uint32_t i = (uint32_t)p / 4096;

    bitmap[i / 32] &= ~(1 << (i % 32));

    if (i < last_index) last_index = i; // update offset
}

/**
 * pgsalloc
 * 
 * Note:
 *  Allocates a contiguous block of pages. Use this
 *  function to allocate memory for DMA transfers.
 * 
 * FIXME: fix pblock size
*/

void *pgsalloc(uint32_t n) {
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
                        return (void *)(i * 16 * 4096); // ### THIS ###
                    } // else...

                    // mov ecx, uint32_t [pages_count]
                    // dec ecx
                    // mov eax, 0x00008000
                    // sar eax, cl
                    if (i > 0 && !(bitmap[i - 1] & ~(((uint16_t)0x8000 >> (pages_count - 1)) - 1))) {
                        // TODO: allocate, change offset
                        return (void *)((i * 16 - pages_count) * 4096);
                    }
                    
                    // mov ecx, uint32_t [pages_count]
                    // xor eax, eax
                    // inc eax
                    // shl eax, cl
                    // dec eax
                    uint32_t l = ++j/* + 1*/; // ### HERE ###
                    if (l < size && !(bitmap[l] & (((uint16_t)0x0001 << pages_count) - 1))) {
                        // TODO: allocate, change offset
                        return (void *)(i * 16 * 4096); // TODO: merge with ### THIS ###?
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
                        return (void *)((i * 16 + j) * 4096);
                    }
                }
            }
        }
    }

    return NULL; // out of memory, change offset(?)
}