/**
 * String
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "limits.h"
#include "bool.h"

/**
 * Declarations
*/

uint32_t strlen(char const *s);
char *strcpy(char *d, char const *s);
char *strncpy(char *d, char const *s, uint32_t n);
void *memset(void *ptr, int value, uint32_t n);
void *memcpy(void *destination, void const *source, uint32_t n);