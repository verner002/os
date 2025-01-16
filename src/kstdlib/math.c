/**
 * Mathematics
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kstdlib/math.h"

/**
 * log10
*/

uint32_t log10(uint32_t i) {
    uint8_t digits[] = { 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10 };
    uint32_t powers[] = { 0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
    uint32_t l;

    asm (
        "push eax\t\n"
        "bsr eax, ebx\t\n"
        "movzx eax, BYTE PTR [ecx+eax+1]\t\n"
        "cmp ebx, DWORD PTR [edx+eax*4]\t\n"
        "sbb al, 0\t\n"
        "mov %0, eax\t\n"
        "pop eax"
        : "=m" (l)
        : "b" (i), "c" (digits), "d" (powers)
        :
    );

    return l;
}