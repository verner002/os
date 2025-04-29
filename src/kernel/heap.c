/**
 * Heap
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/heap.h"

/**
 * Types Definitions
*/

typedef struct __chunk CHUNK;

/**
 * Structures
*/

struct __chunk {
    // uint32_t magic = 0x0badbabe
    bool free;
    uint32_t size;
    CHUNK
        *previous_chunk,
        *next_chunk;
};

/**
 * Static Global Variables
*/

static CHUNK *first_free_chunk = NULL;

/**
 * __init_heap
*/

void __init_heap(void *p, uint32_t s) {
    CHUNK *heap = (CHUNK *)p;
    heap->free = TRUE;
    heap->size = s - sizeof(CHUNK); // `s' is real size of area reserved for heap
    heap->previous_chunk = NULL;
    heap->next_chunk = NULL;

    first_free_chunk = heap;
}

/**
 * kmalloc
 * 
 * NOTE: use AVL tree instead of linked-list?
*/

void *kmalloc(uint32_t n) {
    if (!n) return NULL;

    CHUNK
        *previous_chunk = NULL,
        *chunk = (CHUNK *)first_free_chunk,
        *next_chunk = NULL;

    while (chunk) {
        if (chunk->free && chunk->size >= n) {
            uint32_t remainder = chunk->size - n;
            
            if (remainder > sizeof(CHUNK)) { // split chunk, at least 1 uint8_t can be used
                next_chunk = (CHUNK *)chunk + n;
                next_chunk->free = TRUE;
                next_chunk->size = remainder - sizeof(CHUNK); // usable size, without metadata
                next_chunk->previous_chunk = chunk;
                next_chunk->next_chunk = NULL;
            }

            first_free_chunk = next_chunk;

            chunk->free = FALSE;
            chunk->size = n;
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
 * krealloc
 * 
 * RFC: does this work? (i can't remember) :-D
 * TODO: some parts of code looks similar, merge them if possible
*/

void *krealloc(void *p, uint32_t n) {
    // null ptr or zero size
    if (!p || !n)
        return p; // do nothing (we don't want to take the system down :-P)

    CHUNK
        *current_chunk = (CHUNK *)(p - sizeof(CHUNK)),
        *previous_chunk = current_chunk->previous_chunk,
        *next_chunk = current_chunk->next_chunk;
    
    uint32_t current_chunk_size = current_chunk->size;

    if (n > current_chunk_size) { // we're going to grow in size
        uint32_t required_bytes = n - current_chunk_size;

        bool previous_chunk_check =
            previous_chunk &&
            previous_chunk->free;

        bool next_chunk_check =
            next_chunk &&
            next_chunk->free;

        // TODO: merge _chunk && _chunk->size (+ sizeof(CHUNK)) part
        if (previous_chunk_check && previous_chunk->size > required_bytes) {
            CHUNK *old_current_chunk = current_chunk;
            current_chunk = (void *)current_chunk - required_bytes;
            current_chunk->size += required_bytes;
            // move the current chunk
            memmove(current_chunk, old_current_chunk, old_current_chunk->size);
            previous_chunk->next_chunk = current_chunk;
            previous_chunk->size -= required_bytes;
            p = (void *)current_chunk + sizeof(CHUNK);
        } else if (previous_chunk_check && (previous_chunk->size + sizeof(CHUNK)) >= required_bytes) {
            uint32_t new_size = previous_chunk->size + sizeof(CHUNK);
            memcpy(previous_chunk, current_chunk, current_chunk->size);
            current_chunk = previous_chunk;
            current_chunk->size += new_size;
            next_chunk->previous_chunk = current_chunk;
            p = (void *)current_chunk + sizeof(CHUNK);
        } else if (next_chunk_check && next_chunk->size >= required_bytes) {
            CHUNK *old_next_chunk = next_chunk;
            next_chunk = (void *)next_chunk + required_bytes;
            next_chunk->size -= required_bytes;
            memmove(next_chunk, old_next_chunk, next_chunk->size);
            current_chunk->next_chunk = next_chunk;
            current_chunk->size += required_bytes;
        } else if (next_chunk_check && next_chunk->size + sizeof(CHUNK) >= required_bytes) {
            current_chunk->size += next_chunk->size + sizeof(CHUNK);
            next_chunk->next_chunk->previous_chunk = current_chunk;
            current_chunk->next_chunk = next_chunk->next_chunk;
        } else {
            void *old_p = p;
            p = (void *)kmalloc(n);
            uint32_t chunk_size = current_chunk->size;
            memcpy(p, old_p, n > chunk_size ? chunk_size : n);
            kfree(old_p);
        }

        p = (void *)current_chunk + sizeof(CHUNK);
    } else if (n < current_chunk_size) { // let's shrink the chunk
        // if we cannot split the chunk,
        // let's just return the 'p' and
        // pretend the chunk is smaller
        // everything else would corrupt
        // the heap
        uint32_t free_bytes = current_chunk_size - n; // number of free bytes we can use

        if (previous_chunk && previous_chunk->free) { // we can enlarge the previous chunk
            CHUNK *old_current_chunk = current_chunk;            
            current_chunk = (CHUNK *)((void *)current_chunk + free_bytes);
            current_chunk->size -= free_bytes;
            memmove(current_chunk, old_current_chunk, current_chunk->size);
            // update references
            previous_chunk->next_chunk = current_chunk;
            previous_chunk->size += free_bytes;
        } else if (next_chunk && next_chunk->free) { // we can enlarge the next chunk
            CHUNK *old_next_chunk = current_chunk;
            next_chunk = (CHUNK *)((void *)next_chunk - free_bytes);
            next_chunk->size += free_bytes;
            memmove(next_chunk, old_next_chunk, old_next_chunk->size);
            // update reference and size
            current_chunk->next_chunk = next_chunk;
            current_chunk->size -= free_bytes;
        } else if (free_bytes > sizeof(CHUNK)) { // we can split the current chunk
            // create new chunk following the current one
            CHUNK *new_chunk = (void *)current_chunk + current_chunk->size - free_bytes;
            new_chunk->free = TRUE;
            new_chunk->size = free_bytes - sizeof(CHUNK);
            // update references and size
            new_chunk->next_chunk = current_chunk->next_chunk;
            new_chunk->previous_chunk = current_chunk;
            new_chunk->next_chunk->previous_chunk = new_chunk;
            current_chunk->next_chunk = new_chunk;
            current_chunk->size -= free_bytes; // new size
        } // else ... do nothing
    }
    
    return p;
}

/**
 * kfree
*/

void kfree(void *p) {
    if (!p) return;

    CHUNK *chunk = (CHUNK *)p - sizeof(CHUNK);

    if (chunk->free) return;

    // TODO: merge free chunks, set `first_free_chunk' if `chunk' < `first_free_chunk'
}