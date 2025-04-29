/**
 * Physical Memory Manager (Page Frame Allocator, Pager)
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "null.h"
#include "types.h"
#include "kstdlib/stdio.h"

/**
 * Declarations
*/

int32_t __init_pager(void);
void pgreserve(void *p);
void *pgalloc(void);
void pgfree(void *);
void *pgsalloc(uint32_t n);