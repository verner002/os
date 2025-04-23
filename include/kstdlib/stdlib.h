/**
 * Standard Library
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
#include "string.h"

/**
 * Declarations
*/

void __init_heap(void *p, uint32_t s);
void *malloc(uint32_t s);
void *realloc(void *p, uint32_t n);
void free(void *p);
void qsort(void *array, uint32_t array_size, int element_size, int (*compare)(void const *element1, void const *element2));
int atoi(char const *s);