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

/**
 * Declarations
*/

void __init_pager(dword *bitmap_ptr, unsigned int pages_count);
void *pgalloc(void);
void pgfree(void *);
void *pgsalloc(unsigned int n);