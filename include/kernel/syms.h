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
    byte symbol_name[8];
    dword symbol_value;
    word section_number;
    word symbol_type;
    byte storage_class;
    byte auxiliary_count;
};