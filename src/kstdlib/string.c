/**
 * String
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "types.h"
#include "kstdlib/string.h"

/**
 * strlen
*/

uint32_t strlen(char const *s) {
    uint32_t i = 0;

    while (s[i++]);

    return i;
}

/**
 * strcpy
*/

char *strcpy(char *d, char const *s) {
    for (uint32_t i = 0; (d[i] = s[i]); ++i);

    return d;
}

/**
 * strncpy
*/

char *strncpy(char *d, char const *s, uint32_t n) {
    bool t = TRUE;

    for (uint32_t i = 0; i < n; ++i) d[i] = t = t ? s[i] : '\0';

    return d;
}

/**
 * strcmp
*/

int32_t strcmp(char const *s1, char const *s2) {
    int32_t c;
    
    do
        c = *s1 - *s2;
    while (!c && *s1++ && *s2++);

    return c;
}

/**
 * strncmp
*/

int32_t strncmp(char const *str1, char const *str2, uint32_t n) {
    int32_t c = 0;

    for (uint32_t i = 0; i < n && !c; ++i) c = str1[i] - str2[i];

    return c;
} 

/**
 * memset
*/

void *memset(void *ptr, int value, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i) ((uint8_t *)ptr)[i] = (uint8_t)value;

    return ptr;
}

/**
 * memcpy
*/

void *memcpy(void *destination, void const *source, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i) ((uint8_t *)destination)[i] = ((uint8_t *)source)[i];

    return destination;
}

/**
 * memmove
*/

void *memmove(void *destination, void const *source, uint32_t n) {
    uint8_t *buffer = e820_malloc(n); // TODO: call __kmalloc

    if (buffer) {
        memcpy(buffer, source, n);
        memcpy(destination, buffer, n);
        // e820_free(buffer), leaks memory now
    }

    return destination;
}