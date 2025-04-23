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
 * malloc
 * 
 * NOTE: use AVL tree instead of linked-list?
*/

void *malloc(uint32_t s) {
    if (!s) return NULL;

    CHUNK
        *previous_chunk = NULL,
        *chunk = (CHUNK *)first_free_chunk,
        *next_chunk = NULL;

    while (chunk) {
        if (chunk->free && chunk->size >= s) {
            uint32_t remainder = chunk->size - s;
            
            if (remainder > sizeof(CHUNK)) { // split chunk, at least 1 uint8_t can be used
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
 * realloc
 * 
 * TODO: some parts of code looks similar, merge them if possible
*/

void *realloc(void *p, uint32_t n) {
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
            p = (void *)malloc(n);
            uint32_t chunk_size = current_chunk->size;
            memcpy(p, old_p, n > chunk_size ? chunk_size : n);
            free(old_p);
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
 * free
*/

void free(void *p) {
    if (!p) return;

    CHUNK *chunk = (CHUNK *)p - sizeof(CHUNK);

    if (chunk->free) return;

    // TODO: merge free chunks, set `first_free_chunk' if `chunk' < `first_free_chunk'
}

/**
 * qsort
*/

static void __swap(void *array, int element_size, int i, int j) {
    if (i == j) return; // in-place swap using xor when i==j would result in 0
    
    uint8_t *i_pieces = array + element_size * i;
    uint8_t *j_pieces = array + element_size * j;
    
    for (int k = 0; k < element_size; ++k) { // in-place swap all pieces
        i_pieces[k] ^= j_pieces[k];
        j_pieces[k] ^= i_pieces[k];
        i_pieces[k] ^= j_pieces[k];
    }
}

static int __partition(void *array, int element_size, int (*compare)(void const *element1, void const *element2), int low, int high) {
    void *pivot = array + element_size * low;
    int i = low, j = high;

    while (i < j) {
        while (compare(array + element_size * i, pivot) <= 0 && i <= high - 1) ++i;
        while (compare(array + element_size * j, pivot) > 0 && j >= low + 1) --j;

        if (i < j) __swap(array, element_size, i, j);
    }

    /*if (i != j)*/ __swap(array, element_size, low, j);

    return j;
}

static void __qsort(void *array, int element_size, int (*compare)(void const *element1, void const *element2), int low, int high) {
    if (low >= high || low < 0) return;
    
    int p = __partition(array, element_size, compare, low, high);

    __qsort(array, element_size, compare, low, p - 1);
    __qsort(array, element_size, compare, p + 1, high);
}

void qsort(void *array, uint32_t array_size, int element_size, int (*compare)(void const *element1, void const *element2)) {
    __qsort(array, element_size, compare, 0, array_size - 1);
}

/**
 * atoi
*/

int atoi(char const *s) {
    if (!s) return 0;

    while (*s == ' ') ++s; // skip whitespaces

    int minus = *s == '-';

    if (*s == '+' || minus) ++s;

    int value = 0;

    for (char c; (c = *s) >= '0' && c <= '9'; ++s) value = 10 * value + c - '0';

    return minus ? -value : value;
}