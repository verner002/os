/**
 * @file paging.h
 * @author verner002
 * @date 24/05/2026
*/

#pragma once

#include "kernel/types.h"

typedef uint32_t cr3_t;

#define PAGE_SIZE               (4096)

#define PAGE_NOT_PRESENT        (0)
#define PAGE_PRESENT            (1)
#define PAGE_READ_ONLY          (0)
#define PAGE_READ_WRITE         (2)
#define PAGE_SUPERVISOR         (0)
#define PAGE_USER               (4)
#define PAGE_WRITE_THROUGH      (0)
#define PAGE_WRITE_BACK         (8)
#define PAGE_CACHED             (0)
#define PAGE_DIRECT             (16)

int join_ptable(cr3_t cr3, int index, void *ptable, unsigned int flags);
int map_page(cr3_t cr3, void *vma, void *pma, unsigned int flags);