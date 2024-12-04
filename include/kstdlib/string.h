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

unsigned int strlen(char const *s);
char *strcpy(char *d, char const *s);
char *strncpy(char *d, char const *s, unsigned int n);
void *memcpy(void *destination, void const *source, unsigned int n);