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