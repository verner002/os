/**
 * @file string.c
 * @author verner002
 * @date 25/08/2025
*/

#include "types.h"
#include "kstdlib/string.h"
#include "mm/heap.h"

/**
 * strlen
*/

uint32_t strlen(char const *s) {
    uint32_t l = 0;

    while (*s++)
        ++l;

    return l;
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

    for (uint32_t i = 0; i < n; ++i)
        d[i] = t = t ? s[i] : '\0';

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

    for (uint32_t i = 0; i < n && !c; ++i)
        c = str1[i] - str2[i];

    return c;
} 

/**
 * memset
*/

void *memset(void *ptr, int value, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i)
        ((uint8_t *)ptr)[i] = (uint8_t)value;

    return ptr;
}

/**
 * memcpy
*/

void *memcpy(void *destination, void const *source, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i)
        ((uint8_t *)destination)[i] = ((uint8_t *)source)[i];

    return destination;
}

/**
 * memmove
*/

void *memmove(void *destination, void const *source, uint32_t n) {
    uint8_t *buffer = (uint8_t *)kmalloc(sizeof(uint8_t) * n);

    if (buffer) {
        memcpy(buffer, source, n);
        memcpy(destination, buffer, n);
        kfree(buffer);
    }

    return destination;
}

/**
 * peek
*/

char *peek(char *str, char const *delimiters, bool eq) {
    char ch;
    
    while ((ch = *str)) {
        char const *ds = delimiters;
        char d;

        while ((d = *ds++))
            if (d == ch)
                break;

        if ((d == ch) ^ !eq)
            return str;

        
        ++str;
    }

    return str;
}

/**
 * strtok
 * 
 * NOTE: not thread-safe
*/

char *strtok(char *str, char const *delimiters) {
    static char *last;

    if (!str)
        str = last;

    // find first char that is not delim
    char *start = peek(str, delimiters, FALSE);
    // find first char that is delim
    char *end = peek(start, delimiters, TRUE);

    // we've reached the end
    if (start == end)
        return NULL;

    last = end;

    // continue with next symbol
    if (*end) {
        ++last;
        *end = '\0';
    }

    return start;
}