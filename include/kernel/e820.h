/**
 * @file e820.h
 * @author verner002
 * @date 23/09/2025
*/

#pragma once

#include <stddef.h>

#include <kernel/types.h>

#define E820_MAX_ENTRIES 128

struct __attribute__((packed)) e820_entry {
    uint32_t base;
    uint32_t size;
    uint32_t type;
};

struct e820_map {
    size_t nentries;
    struct e820_entry entries[E820_MAX_ENTRIES];
};

void e820_sanitize(size_t e820_nentries, struct e820_entry *e820_entries);
void e820_dump_mmap(void);
void *e820_alloc(uint32_t n, int a, uint32_t l);
void *e820_malloc(uint32_t n);
void *e820_rmalloc(uint32_t n, int a);