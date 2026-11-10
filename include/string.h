/**
 * @file string.h
 * @author verner002
 * @date 08/09/2025
*/

#pragma once

#include <stddef.h>

size_t strlen(char const *str);
char *strcpy(char *dest, char const *src);
char *strncpy(char *dest, char const *src, size_t count);
int strcmp(char const *lhs, char const *rhs);
char *strchr(char const *str, int c);
int strncmp(char const *str1, char const *str2, size_t count);
void *memset(void *dest, int c, size_t count);
void *memcpy(void *dest, void const *src, size_t count);
void *memchr(void const *src, int c, size_t count);
char *strtok_r(char *str, char const *delim, char **saveptr);
char *strtok(char *str, const char *delim);
size_t strcspn(char const *str, char const *reject);