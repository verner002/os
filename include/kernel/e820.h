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

typedef enum __region_type REGION_TYPE;
typedef struct __address_descriptor ADDRESS_DESCRIPTOR;
typedef struct _e820_entry E820_ENTRY;
typedef struct _e820_map E820_MAP;

/**
 * Enumerations
*/

enum __region_type {
    REGION_UNKNOWN = 0,
    REGION_FREE = 1,
    REGION_RESERVED = 2,
    REGION_ACPI_RECLAIMABLE = 3,
    REGION_ACPI_NVS = 4,
    REGION_BAD = 5
};

/**
 * Structures
*/

struct __address_descriptor {
    uint32_t address;
    E820_ENTRY *entry;
};

struct __attribute__((__packed__)) _e820_entry {
    uint32_t base;
    uint32_t size;
    uint32_t type; // TODO: use REGION_TYPE..?
};

struct __attribute__((__packed__)) _e820_map {
    uint32_t index;
    E820_ENTRY entries[E820_MAX_ENTRIES];
};

void __add_region(E820_ENTRY entry);
void __insert_region(E820_ENTRY entry, uint32_t index);
void __sanitize_e820(uint32_t count, E820_ENTRY *map);
E820_ENTRY *__get_last_entry(void);
char const *e820_get_type_string(E820_ENTRY *descriptor);
void __dump_e820(void);
void *e820_amalloc(uint32_t n, bool a);
void *e820_malloc(uint32_t n);
void *e820_rmalloc(uint32_t n, bool a);
E820_ENTRY *e820_get_descriptor(uint32_t index);