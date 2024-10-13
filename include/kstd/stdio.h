/**
 * Standard Input/Output
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "kstd/types.h"

/**
 * Types Definitions
*/

typedef struct _file FILE;

/**
 * Structures
*/

struct _file {
    unsigned int index;
};

/**
 * Declarations
*/

void printf(char const *s, ...);