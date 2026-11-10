/**
 * @file string.c
 * @author verner002
 * @date 08/09/2025
*/

#include <string.h>

int strncmp(char const *lhs, char const *rhs, size_t count) {
    int diff = 0;

    for (size_t i = 0; i < count && !diff; ++i)
        diff = lhs[i] - rhs[i];

    return diff;
}

static inline char *peek(char *str, char const *delim, int eq) {
    char ch;
    
    while ((ch = *str)) {
        char const *ds = delim;
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

char *strtok_r(char *str, char const *delim, char **saveptr) {
    if (!str)
        str = *saveptr;

    // find first char that is not delim
    char *start = peek(str, delim, 0);
    // find first char that is delim
    char *end = peek(start, delim, 1);

    // we've reached the end
    if (start == end)
        return NULL;

    *saveptr = end;

    // continue with next symbol
    if (*end) {
        ++*saveptr;
        *end = '\0';
    }

    return start;
}

char *strtok(char *str, const char *delim) {
    static char *last;

    return strtok_r(str, delim, &last);    
}

size_t strcspn(char const *str, char const *reject) {
    size_t size = 0;

    for (char const *p = str; *p; ++p) {
        for (char const *r = reject; *r; ++r)
            if (*p == *r)
                return size;

        ++size;
    }

    return size;
}