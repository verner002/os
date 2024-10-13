/**
 * Standard Input/Output
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kstd/types.h"
#include "kstd/stdio.h"

/**
 * Static Global Variables
*/

static char *const video_memory = (char *)0x000b800;

/**
 * _stack_chk_fail
*/

void __stack_chk_fail(void) {
    printf("Stack Overflow\n");
}

/**
 * putchr
*/

void putchr(FILE file, char chr) {
    /*if (file == STDOUT) {
        video_memory[0] = chr;
    } else if (file == STDERR) {
        
    }*/
}

/**
 * printf
*/

void printf(char const *s, ...) {
    va_list args;

    va_start(args, s);

    va_end(args);
}