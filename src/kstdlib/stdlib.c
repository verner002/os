/**
 * Standard Library
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kstdlib/stdlib.h"

/**
 * Static Global Variables
*/

static CHUNK *first_free_chunk = NULL;

/**
 * init_heap
*/

void init_heap(byte *p, unsigned int s) {
    CHUNK *heap = (CHUNK *)p;
    heap->free = TRUE;
    heap->size = s - sizeof(CHUNK); // `s' is real size of area reserved for heap
    heap->previous_chunk = NULL;
    heap->next_chunk = NULL;

    first_free_chunk = heap;
}

/**
 * malloc
 * 
 * NOTE: use AVL tree instead of linked-list?
*/

void *malloc(unsigned int s) {
    if (!s) return NULL;

    CHUNK
        *previous_chunk = NULL,
        *chunk = (CHUNK *)first_free_chunk,
        *next_chunk = NULL;

    while (chunk) {
        if (chunk->free && chunk->size >= s) {
            unsigned int remainder = chunk->size - s;
            
            if (remainder > sizeof(CHUNK)) { // split chunk, at least 1 byte can be used
                next_chunk = (CHUNK *)chunk + s;
                next_chunk->free = TRUE;
                next_chunk->size = remainder - sizeof(CHUNK); // usable size, without chunk struct
                next_chunk->previous_chunk = chunk;
                next_chunk->next_chunk = NULL;
            }

            first_free_chunk = next_chunk;

            chunk->free = FALSE;
            chunk->size = s;
            chunk->previous_chunk = previous_chunk;
            chunk->next_chunk = next_chunk;

            return chunk + sizeof(CHUNK);
        }

        previous_chunk = chunk;
        chunk = chunk->next_chunk;
    }

    return NULL; // out of memory
}

/**
 * free
*/

void free(void *p) {
    if (!p) return;

    CHUNK *chunk = (CHUNK *)p - sizeof(CHUNK);

    if (chunk->free) return;

    // TODO: merge free chunks, set `first_free_chunk' if `chunk' < `first_free_chunk'
}