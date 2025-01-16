/**
 * Symbols
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"

/**
 * Types Definitions
*/

typedef struct _symbol SYMBOL;

/**
 * Structures
*/

struct __attribute__((__packed__)) _symbol {
    uint8_t symbol_name[8];
    uint32_t symbol_value;
    uint16_t section_number;
    uint16_t symbol_type;
    uint8_t storage_class;
    uint8_t auxiliary_count;
};