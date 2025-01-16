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

typedef struct __address_descriptor ADDRESS_DESCRIPTOR;
typedef struct _e820_entry E820_ENTRY;
typedef struct _e820_map E820_MAP;

/**
 * Structures
*/

struct __address_descriptor {
    uint32_t address;
    E820_ENTRY *entry;
    bool end;
};

struct __attribute__((__packed__)) _e820_entry {
    uint32_t base;
    uint32_t size;
    uint32_t type;
};

struct __attribute__((__packed__)) _e820_map {
    uint32_t index;
    E820_ENTRY entries[E820_MAX_ENTRIES];
};

void __init_e820(uint32_t entries_count, E820_ENTRY *map);
char const *e820_get_type_string(E820_ENTRY *descriptor);
void dump_e820(void);
void *e820_alloc(uint32_t n);
E820_ENTRY *e820_get_descriptor(uint32_t index);