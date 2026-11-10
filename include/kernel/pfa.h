/**
 * @file pfa.h
 * @author verner002
 * @date 23/09/2025
*/

#pragma once

#include <kernel/e820.h>

int pfa_init(struct e820_map *mmap);
void pfa_reserve(void *ptr);
void *pfa_alloc(void);
void pfa_dealloc(void *ptr);