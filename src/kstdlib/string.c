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

unsigned int strlen(char const *s) {
    unsigned int i = __UINT_MAX;

    while (s[++i]); // `i' wraps to `0' after first iteration

    return i;
}

/**
 * strcpy
*/

char *strcpy(char *d, char const *s) {
    for (unsigned int i = 0; (d[i] = s[i]); ++i);

    return d;
}

/**
 * strncpy
*/

char *strncpy(char *d, char const *s, unsigned int n) {
    bool t = TRUE;

    for (unsigned int i = 0; i < n; ++i) d[i] = t = t ? s[i] : '\0';

    return d;
}

/**
 * memcpy
*/

void *memcpy(void *destination, void const *source, unsigned int n) {
    for (unsigned int i = 0; i < n; ++i) ((byte *)destination)[i] = ((byte *)source)[i];

    return destination;
}