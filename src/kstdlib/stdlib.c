/**
 * Standard Library
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/heap.h"

/**
 * qsort
*/

static void __swap(void *array, int element_size, int i, int j) {
    if (i == j) return; // in-place swap using xor when i==j would result in 0
    
    uint8_t *i_pieces = array + element_size * i;
    uint8_t *j_pieces = array + element_size * j;
    
    for (int k = 0; k < element_size; ++k) { // in-place swap all pieces
        i_pieces[k] ^= j_pieces[k];
        j_pieces[k] ^= i_pieces[k];
        i_pieces[k] ^= j_pieces[k];
    }
}

static int __partition(void *array, int element_size, int (*compare)(void const *element1, void const *element2), int low, int high) {
    void *pivot = array + element_size * low;
    int i = low, j = high;

    while (i < j) {
        while (compare(array + element_size * i, pivot) <= 0 && i <= high - 1) ++i;
        while (compare(array + element_size * j, pivot) > 0 && j >= low + 1) --j;

        if (i < j) __swap(array, element_size, i, j);
    }

    /*if (i != j)*/ __swap(array, element_size, low, j);

    return j;
}

static void __qsort(void *array, int element_size, int (*compare)(void const *element1, void const *element2), int low, int high) {
    if (low >= high || low < 0) return;
    
    int p = __partition(array, element_size, compare, low, high);

    __qsort(array, element_size, compare, low, p - 1);
    __qsort(array, element_size, compare, p + 1, high);
}

void qsort(void *array, uint32_t array_size, int element_size, int (*compare)(void const *element1, void const *element2)) {
    __qsort(array, element_size, compare, 0, array_size - 1);
}

/**
 * atoi
*/

int atoi(char const *s) {
    if (!s) return 0;

    while (*s == ' ') ++s; // skip whitespaces

    int minus = *s == '-';

    if (*s == '+' || minus) ++s;

    int value = 0;

    for (char c; (c = *s) >= '0' && c <= '9'; ++s)
        value = 10 * value + c - '0';

    return minus ? -value : value;
}