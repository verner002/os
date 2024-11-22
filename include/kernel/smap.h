/**
 * System Memory Map
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

typedef struct _smap_entry SMAP_ENTRY;

/**
 * Structures
*/

struct _smap_entry {
    dword base;
    dword size;
    dword type;
};