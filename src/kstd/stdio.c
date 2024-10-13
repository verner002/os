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
 * putchr
*/

void putchr(FILE file, char chr) {
    if (file == STDOUT) {
        video_memory[0] = chr;
    } else if (file == STDERR) {
        
    }
}