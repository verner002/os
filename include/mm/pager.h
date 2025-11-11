/**
 * Physical Memory Manager (Page Frame Allocator, Pager)
 * 
 * Author: verner002
*/

#pragma once

#include "null.h"
#include "types.h"
#include "kstdlib/stdio.h"

#define PAGE_NO_MAP 0
#define PAGE_MAP 1

int32_t __init_pager(void);
void pgreserve(void *p);
void *pgalloc(uint32_t flags);
void pgfree(void *);
void *pgsalloc(uint32_t n);