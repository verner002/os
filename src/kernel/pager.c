/**
 * Pager
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/pager.h"

static dword *bitmap; // max 128 KiB to represent 4 GiB
static dword size; // max 1024*1024/16 pages to represent 4 GiB
static unsigned int offset; // first free page in bitmap

/**
 * init_pager
*/

void init_pager(dword *bitmap_ptr, unsigned int pages_count) {
    if (pages_count % 16 || pages_count < 32) {
        //error();
        return;
    }

    bitmap = bitmap_ptr;
    size = pages_count / 16; // 16 pages per dword
    //offset = 0;

    for (unsigned int i = 1; i < size; ++i) bitmap[i] = 0;

    bitmap[0] = 0x0001; // NULL cannot be assigned

    // TODO: allocate memory occupied by bitmap here?
}

/**
 * kalloc
*/

void *kalloc(unsigned int n) {
    if (!n) return NULL;

    unsigned int chunks_count = n / 16; // number of words
    unsigned int pages_count = n % 16; // number of bits

    if (chunks_count) { // handles allocation of large blocks of memory
        unsigned int last_chunk = size - chunks_count + 1;
        unsigned int chunks_to_find = chunks_count;
        unsigned int i = 0;

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
                        // allocate, change offset
                        return (void *)(i * 16 * 4096); // ### THIS ###
                    } // else...

                    // mov ecx, dword [pages_count]
                    // dec ecx
                    // mov eax, 0x00008000
                    // sar eax, cl
                    if (i > 0 && !(bitmap[i - 1] & ~(((word)0x8000 >> (pages_count - 1)) - 1))) {
                        // allocate, change offset
                        return (void *)((i * 16 - pages_count) * 4096);
                    }
                    
                    // mov ecx, dword [pages_count]
                    // xor eax, eax
                    // inc eax
                    // shl eax, cl
                    // dec eax
                    unsigned int l = ++j/* + 1*/; // ### HERE ###
                    if (l < size && !(bitmap[l] & (((word)0x0001 << pages_count) - 1))) {
                        // allocate, change offset
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
                        // allocate, change offset
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