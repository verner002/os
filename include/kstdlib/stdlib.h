/**
 * Standard Library
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "null.h"
#include "types.h"
#include "bool.h"

/**
 * Types Definitions
*/

typedef struct _chunk CHUNK;

/**
 * Structures
*/

struct _chunk {
    // uint32_t magic = 0x0badbabe
    bool free;
    uint32_t size;
    CHUNK
        *previous_chunk,
        *next_chunk;
};

/**
 * Declarations
*/

void __init_heap(void *p, uint32_t s);
void *malloc(uint32_t s);
void free(void *p);
void qsort(void *array, uint32_t array_size, int element_size, int (*compare)(void const *element1, void const *element2));
int atoi(char const *s);