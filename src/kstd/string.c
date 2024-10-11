/**
 * String
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kstd/bool.h"

/**
 * String Copy
*/

char *strcpy(char *d, char const *s) {
    unsigned int i = 0;
    
    while (d[i++] = s[i]);

    return d;
}

/**
 * String N Copy
*/

char *strncpy(char *d, char const *s, unsigned int n) {
    unsigned int i = 0;
    bool t = TRUE;

    while (i < n) d[i++] = t = t ? s[i] : '\0';

    return d;
}