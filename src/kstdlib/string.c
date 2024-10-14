/**
 * String
 * 
 * Author: verner002
*/

/**
 * Includes
*/

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
    unsigned int i = 0;
    
    while ((d[i++] = s[i]));

    return d;
}

/**
 * strncpy
*/

char *strncpy(char *d, char const *s, unsigned int n) {
    unsigned int i = 0;
    bool t = TRUE;

    while (i < n) d[i++] = t = t ? s[i] : '\0';

    return d;
}