/**
 * @file e820.h
 * @author verner002
 * @date 23/09/2025
*/

#pragma once

#include "types.h"
#include "bool.h"

#define E820_MAX_ENTRIES 128

typedef enum __region_type REGION_TYPE;
typedef struct _e820_entry E820_ENTRY;
typedef struct _e820_map E820_MAP;

enum __region_type {
    REGION_UNKNOWN = 0,
    REGION_FREE = 1,
    REGION_RESERVED = 2,
    REGION_ACPI_RECLAIMABLE = 3,
    REGION_ACPI_NVS = 4,
    REGION_BAD = 5
};

struct __attribute__((__packed__)) _e820_entry {
    uint32_t base;
    uint32_t size;
    uint32_t type; // TODO: use REGION_TYPE..?
};

struct __attribute__((__packed__)) _e820_map {
    uint32_t count;
    E820_ENTRY entries[E820_MAX_ENTRIES];
};

extern E820_MAP smap;

void __e820_sanitize(uint32_t count, E820_ENTRY *map);
void __e820_dump_mmap(void);
void *__e820_alloc(uint32_t n, bool a, bool f);
void *__e820_malloc(uint32_t n);
void *__e820_rmalloc(uint32_t n, bool a);