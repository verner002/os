/**
 * Heap
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/heap.h"
#include "kstdlib/stdio.h"

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

static bool mutex = FALSE;
static bool init = FALSE;
static CHUNK *first_chunk = NULL;
static CHUNK *first_free_chunk = NULL;

/**
 * __init_heap
*/

void __init_heap(void *p, uint32_t s) {
    if (init)
        return;

    __mutex_lock(&mutex);
    CHUNK *heap = (CHUNK *)p;
    heap->free = TRUE;
    heap->size = s - sizeof(CHUNK); // `s' is real size of area reserved for heap
    heap->previous_chunk = NULL;
    heap->next_chunk = NULL;

    first_free_chunk = heap;
    first_chunk = heap;
    init = TRUE;
    __mutex_unlock(&mutex);
}

/**
 * __dump_heap
*/

void __dump_heap(void) {
    __mutex_lock(&mutex);
    CHUNK *chunk = first_chunk;
    uint32_t chunk_i = 0;

    while (chunk) {
        printk("chunk=%u | ptr=%p | free=%u | size=%u\n", chunk_i++, chunk, chunk->free, chunk->size);
        chunk = chunk->next_chunk;
    }

    printk("start=%p | total=%u\n", first_chunk, chunk_i);
    __mutex_unlock(&mutex);
}

/**
 * __merge_chunks
*/

static CHUNK *__merge_chunks(CHUNK *chunk) {
    CHUNK *prev_chunk = chunk->previous_chunk;

    if (prev_chunk && prev_chunk->free) {
        prev_chunk->size += chunk->size + sizeof(CHUNK);
        prev_chunk->next_chunk = chunk->next_chunk;

        if (chunk->next_chunk)
            chunk->next_chunk->previous_chunk = prev_chunk;

        chunk = prev_chunk;
    }

    CHUNK *next_chunk = chunk->next_chunk;

    if (next_chunk && next_chunk->free) {
        chunk->size += next_chunk->size + sizeof(CHUNK);
        chunk->next_chunk = next_chunk->next_chunk;

        if (next_chunk->next_chunk)
            next_chunk->next_chunk->previous_chunk = chunk;
    }

    return chunk;
}

/**
 * __kmalloc
 * 
 * NOTE: use AVL tree instead of linked-list?
*/

void *__kmalloc(uint32_t n) {
    if (!n)
        return NULL;

    CHUNK *chunk = (CHUNK *)first_free_chunk;

    while (chunk) {
        if (chunk->free && chunk->size >= n) {
            CHUNK *next_chunk = chunk->next_chunk;

            uint32_t remainder = chunk->size - n;
            
            if (remainder > sizeof(CHUNK)) { // split chunk
                next_chunk = (CHUNK *)chunk + n;
                next_chunk->free = TRUE;
                next_chunk->size = remainder - sizeof(CHUNK); // usable size, without metadata
                next_chunk->previous_chunk = chunk;
                next_chunk->next_chunk = chunk->next_chunk;

                chunk->size = n;
                chunk->next_chunk = next_chunk;
            }

            first_free_chunk = next_chunk;
            chunk->free = FALSE;

            return (void *)((uint32_t)chunk + (uint32_t)sizeof(CHUNK));
        }

        chunk = chunk->next_chunk;
    }

    return NULL; // out of memory
}

/**
 * __kfree
*/

void __kfree(void *p) {
    if (!p)
        return;

    CHUNK *curr_chunk = (CHUNK *)((uint32_t)p - (uint32_t)sizeof(CHUNK));

    if (curr_chunk->free)
        return;

    curr_chunk->free = TRUE;
    curr_chunk = __merge_chunks(curr_chunk);

    if ((uint32_t)curr_chunk < (uint32_t)first_free_chunk)
        first_free_chunk = curr_chunk;
}

/**
 * kmalloc
*/

void *kmalloc(uint32_t n) {
    __mutex_lock(&mutex);
    void *ptr = __kmalloc(n);
    __mutex_unlock(&mutex);
    return ptr;
}

/**
 * krealloc
 * TODO: some parts of code looks similar, merge them if possible
 * FIXME: we must update first_free_chunk
*/

void *krealloc(void *p, uint32_t n) {
    __mutex_lock(&mutex);

    if (!p || !n) {
        __mutex_unlock(&mutex);
        return p;
    }

    CHUNK *curr_chunk = (CHUNK *)((uint32_t)p - (uint32_t)sizeof(CHUNK));

    // you can not realloc something that
    // is not even allocated
    if (curr_chunk->free) {
        __mutex_unlock(&mutex);
        return NULL; // TODO: throw an error?
    }

    uint32_t curr_chunk_size = curr_chunk->size;

    if (n == curr_chunk->size) {
        __mutex_unlock(&mutex);
        return p;
    }

    CHUNK
        *prev_chunk = curr_chunk->previous_chunk,
        *next_chunk = curr_chunk->next_chunk;

    if (n < curr_chunk_size) {
        uint32_t extra_bytes = curr_chunk_size - n;

        // first let's try to enlarge prev or next
        // chunk we want to avoid fragmentation, if
        // that isn't possible we can try to split
        // the chunk into the one we use and into the
        // remainder, if that is not possible, let
        // the chunk as it is, changing the size
        // would corrupt heap
        if (prev_chunk && prev_chunk->free) {
            prev_chunk->size += extra_bytes;
            curr_chunk->size = n;
            CHUNK *new_chunk = (CHUNK *)((uint32_t)curr_chunk + extra_bytes);
            //memmove((void *)new_chunk, (void *)curr_chunk, n + sizeof(CHUNK));

            uint32_t n_total = n + sizeof(CHUNK);

            uint8_t *buffer = (uint8_t *)__kmalloc(sizeof(uint8_t) * n_total);

            if (buffer) {
                memcpy(buffer, (void *)curr_chunk, n_total);
                memcpy((void *)new_chunk, buffer, n_total);
                __kfree(buffer);
            }

            p = (void *)((uint32_t)new_chunk + (uint32_t)sizeof(CHUNK));
            prev_chunk->next_chunk = new_chunk; // update ptr

            if (next_chunk)
                next_chunk->previous_chunk = new_chunk;
        } else if (next_chunk && next_chunk->free) {
            next_chunk->size += extra_bytes;
            curr_chunk->size = n;
            CHUNK *new_chunk = (CHUNK *)((uint32_t)next_chunk - extra_bytes);
            //memmove((void *)new_chunk, (void *)next_chunk, next_chunk->size + sizeof(CHUNK));

            uint32_t n_total = next_chunk->size + sizeof(CHUNK);

            uint8_t *buffer = (uint8_t *)__kmalloc(sizeof(uint8_t) * n_total);

            if (buffer) {
                memcpy(buffer, (void *)next_chunk, n_total);
                memcpy((void *)new_chunk, buffer, n_total);
                __kfree(buffer);
            }

            curr_chunk->next_chunk = new_chunk; // update ptr

            next_chunk = next_chunk->next_chunk;

            if (next_chunk)
                next_chunk->previous_chunk = new_chunk;
        } else if (extra_bytes > sizeof(CHUNK)) {
            curr_chunk->size = n;
            CHUNK *new_chunk = (CHUNK *)((uint32_t)curr_chunk + n);
            new_chunk->free = TRUE;
            new_chunk->size = extra_bytes - sizeof(CHUNK);
            new_chunk->previous_chunk = curr_chunk;
            new_chunk->next_chunk = next_chunk;
            next_chunk->previous_chunk = new_chunk;
            curr_chunk->next_chunk = new_chunk;
        }
    } else/* if (n > curr_chunk_size)*/ {
        uint32_t required_bytes = n - curr_chunk_size;

        bool
            prev_chunk_free = prev_chunk && prev_chunk->free,
            next_chunk_free = next_chunk && next_chunk->free;

        if (next_chunk_free && next_chunk->size > required_bytes) {
            next_chunk->size -= required_bytes;
            curr_chunk->size = n;
            CHUNK *new_chunk = (CHUNK *)((uint32_t)next_chunk + required_bytes);
            //memmove((void *)new_chunk, (void *)next_chunk, /*next_chunk->size + */sizeof(CHUNK));

            uint32_t n_total = sizeof(CHUNK);

            uint8_t *buffer = (uint8_t *)__kmalloc(sizeof(uint8_t) * n_total);

            if (buffer) {
                memcpy(buffer, (void *)next_chunk, n_total);
                memcpy((void *)new_chunk, buffer, n_total);
                __kfree(buffer);
            }

            curr_chunk->next_chunk = new_chunk;

            next_chunk = next_chunk->next_chunk;

            if (next_chunk)
                next_chunk->previous_chunk = new_chunk;
        } else if (next_chunk_free && next_chunk->size + sizeof(CHUNK) >= required_bytes) {
            curr_chunk->size += next_chunk->size + sizeof(CHUNK);
            next_chunk = next_chunk->next_chunk;
            curr_chunk->next_chunk = next_chunk;

            if (next_chunk)
                next_chunk->previous_chunk = curr_chunk;
        } else if (prev_chunk_free && prev_chunk->size > required_bytes) {
            prev_chunk->size -= required_bytes;
            curr_chunk->size = n;
            CHUNK *new_chunk = (CHUNK *)((uint32_t)curr_chunk - required_bytes);
            //memmove((void *)new_chunk, (void *)curr_chunk, curr_chunk->size + sizeof(CHUNK));

            uint32_t n_total = curr_chunk->size + sizeof(CHUNK);

            uint8_t *buffer = (uint8_t *)__kmalloc(sizeof(uint8_t) * n_total);

            if (buffer) {
                memcpy(buffer, (void *)curr_chunk, n_total);
                memcpy((void *)new_chunk, buffer, n_total);
                __kfree(buffer);
            }

            p = (void *)((uint32_t)new_chunk + (uint32_t)sizeof(CHUNK));
            prev_chunk->next_chunk = new_chunk; // update ptr

            if (new_chunk)
                next_chunk->previous_chunk = new_chunk;
        } else if (prev_chunk_free && prev_chunk->size + sizeof(CHUNK) >= required_bytes) {
            curr_chunk->previous_chunk = prev_chunk->previous_chunk;
            uint32_t bonus_bytes = prev_chunk->size + sizeof(CHUNK);
            memcpy((void *)prev_chunk, (void *)curr_chunk, curr_chunk->size + sizeof(CHUNK));
            p = (void *)((uint32_t)curr_chunk + (uint32_t)sizeof(CHUNK));
            curr_chunk = prev_chunk;
            curr_chunk->size += bonus_bytes;

            if (next_chunk)
                next_chunk->previous_chunk = curr_chunk;
        } else {
            void *new_p = __kmalloc(n);

            if (!new_p) {
                __mutex_unlock(&mutex);
                return NULL;
            }

            memcpy(new_p, p, curr_chunk->size);
            __kfree(p);
            p = new_p;
        }
    }

    __mutex_unlock(&mutex);
    return p;
}

/**
 * kfree
*/

void kfree(void *p) {
    __mutex_lock(&mutex);
    __kfree(p);
    __mutex_unlock(&mutex);
}