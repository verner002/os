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
 * Constants
*/

#define va_start(v, l) __builtin_va_start(v, l)
#define va_arg(v, l) __builtin_va_arg(v, l)
#define va_end(v) __builtin_va_end(v)

/**
 * Types Definitions
*/

typedef struct _file FILE;
typedef __builtin_va_list va_list;

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