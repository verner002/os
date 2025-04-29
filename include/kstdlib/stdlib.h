/**
 * Standard Library
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"

/**
 * Declarations
*/

void qsort(void *array, uint32_t array_size, int element_size, int (*compare)(void const *element1, void const *element2));
int atoi(char const *s);