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
#include "kernel/e820.h"

/**
 * Declarations
*/

uint32_t strlen(char const *s);
char *strcpy(char *d, char const *s);
int32_t strcmp(char const *s1, char const *s2);
char *strncpy(char *d, char const *s, uint32_t n);
void *memset(void *ptr, int value, uint32_t n);
void *memcpy(void *destination, void const *source, uint32_t n);
void *memmove(void *destination, void const *source, uint32_t n);
int32_t strncmp(char const *str1, char const *str2, uint32_t n);