/**
 * Heap
 * 
 * Author: verner002
*/

#include "null.h"
#include "types.h"
#include "bool.h"
#include "kstdlib/string.h"

/**
 * Declarations
*/

void __init_heap(void *p, uint32_t s);
void *kmalloc(uint32_t n);
void *krealloc(void *p, uint32_t n);
void kfree(void *p);