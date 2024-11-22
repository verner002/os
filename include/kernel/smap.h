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

typedef struct _address_range_descriptor ADDRESS_RANGE_DESCRIPTOR;

/**
 * Structures
*/

struct _address_range_descriptor {
    dword base;
    dword size;
    dword type;
};