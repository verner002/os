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
    // unsigned int magic = 0x0badbabe
    bool free;
    unsigned int size;
    CHUNK
        *previous_chunk,
        *next_chunk;
};

/**
 * Declarations
*/

void *malloc(unsigned int s);
void free(void *p);
void qsort(void *array, unsigned int array_size, unsigned int element_size, int (*compare)(void const *element1, void const *element2));