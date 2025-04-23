/**
 * Driver
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "null.h"
#include "types.h"
#include "kstdlib/string.h"

/**
 * Types Definitions
*/

typedef struct __driver DRIVER;
typedef struct __symbol SYMBOL;

/**
 * Enumeration
*/

enum __storage_class {
    SYMBOL_STORAGE_CLASS_EXT = 2,
    SYMBOL_STORAGE_CLASS_STAT = 3 
};

/**
 * Structures
*/

struct __driver {
    uint32_t symbols_count;
    SYMBOL *symbols;
};

struct __attribute__((__packed__)) __symbol {
    uint8_t name[32];
    uint32_t address;
};

/**
 * Declarations
*/

uint32_t __link_symbol(DRIVER *driver, char const *name);