/**
 * Physical Memory Manager (Page Frame Allocator, Pager)
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/pager.h"

static dword *bitmap; // max 128 KiB to represent 4 GiB
static dword size; // max 1024*1024/16 pages to represent 4 GiB
static unsigned int last_index;

/**
 * init_pager
*/

void __init_pager(dword *bitmap_ptr, unsigned int pages_count) {
    if (pages_count % 16 || pages_count < 32) {
        //error();
        return;
    }

    bitmap = bitmap_ptr;
    size = pages_count / 16; // 16 pages per dword
    last_index = 0;

    for (unsigned int i = 1; i < size; ++i) bitmap[i] = 0;

    bitmap[0] = 0x0001; // NULL cannot be assigned

    // TODO: allocate memory occupied by bitmap here?
}

/**
 * pgalloc
 * 
 * Note:
 *  Allocates a page.
*/

void *pgalloc(void) {
    for (unsigned int i = last_index; i < size; ++i) {
        if (~bitmap[i]) {
            word temp = bitmap[i];
            temp ^= bitmap[i] |= temp + 1; // toggles the first 0 bit it finds

            asm (
                "push ax\t\n"
                "bsf ax, %1\t\n"
                "mov %0, ax\t\n"
                "pop ax"
                : "=m" (temp)
                : "m" (temp)
                :
            );
            
            // fast log2
            /*register unsigned int r = (temp & 0x0000aaaa) != 0;
            r |= ((temp & 0x0000cccc) != 0) << 1;
            r |= ((temp & 0x0000f0f0) != 0) << 2;
            r |= ((temp & 0x0000ff00) != 0) << 3;*/

            last_index = i; // update offset
            
            return (void *)((i * 16 + (unsigned int)temp) * 4096);
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

    unsigned int i = (unsigned int)p / 4096;

    bitmap[i / 16] &= ~(1 << (i % 16));

    if (i < last_index) last_index = i; // update offset
}

/**
 * pgsalloc
 * 
 * Note:
 *  Allocates a contiguous block of pages. Use this
 *  function to allocate memory for DMA transfers.
*/

void *pgsalloc(unsigned int n) {
    if (!n) return NULL;

    unsigned int chunks_count = n / 16; // number of words
    unsigned int pages_count = n % 16; // number of bits

    if (chunks_count) { // handles allocation of large blocks of memory
        unsigned int last_chunk = size - chunks_count + 1;
        unsigned int chunks_to_find = chunks_count;
        unsigned int i = 0; // TODO: start from offset

        for (; i < last_chunk; ++i) {
            if (!bitmap[i]) {
                unsigned int j = i;

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

                    // mov ecx, dword [pages_count]
                    // dec ecx
                    // mov eax, 0x00008000
                    // sar eax, cl
                    if (i > 0 && !(bitmap[i - 1] & ~(((word)0x8000 >> (pages_count - 1)) - 1))) {
                        // TODO: allocate, change offset
                        return (void *)((i * 16 - pages_count) * 4096);
                    }
                    
                    // mov ecx, dword [pages_count]
                    // xor eax, eax
                    // inc eax
                    // shl eax, cl
                    // dec eax
                    unsigned int l = ++j/* + 1*/; // ### HERE ###
                    if (l < size && !(bitmap[l] & (((word)0x0001 << pages_count) - 1))) {
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
        for (unsigned int i = 0; i < size - 1; ++i) {
            if (~bitmap[i]) {
                dword dchunk = ((dword)bitmap[i + 1] << 16) | bitmap[i];
                dword mask = ((dword)0x00000001 << pages_count) - 1;

                for (unsigned int j = 0; j < 32 - pages_count; ++j) {
                    if (!(dchunk & (mask << j))) {
                        // TODO: change offset
                        bitmap[i] |= (word)(mask << j);
                        bitmap[i + 1] |= (word)((mask << j) >> 16);
                        return (void *)((i * 16 + j) * 4096);
                    }
                }
            }
        }
    }

    return NULL; // out of memory, change offset(?)
}