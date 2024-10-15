/**
 * Standard Library
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "kstdlib/null.h"
#include "kstdlib/types.h"
#include "kstdlib/bool.h"

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