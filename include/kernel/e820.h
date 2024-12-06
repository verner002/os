/**
 * E820
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
#include "kstdlib/stdio.h"
#include "kstdlib/string.h"

/**
 * Constants
*/

#define E820_MAX_ENTRIES 128

/**
 * Types Definitions
*/

typedef struct _e820_entry E820_ENTRY;
typedef struct _e820_map E820_MAP;

/**
 * Structures
*/

struct __attribute__((__packed__)) _e820_entry {
    dword base;
    dword size;
    dword type;
};

struct __attribute__((__packed__)) _e820_map {
    unsigned int index;
    E820_ENTRY entries[E820_MAX_ENTRIES];
};

E820_MAP *__sanitize_e820(unsigned int entries_count, E820_ENTRY *map);
void dump_e820(unsigned int entries_count, E820_ENTRY *map);
void *e820_alloc(unsigned int n);