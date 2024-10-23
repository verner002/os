/**
 * Extended File System 2
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/ext2.h"

/**
 * __load_superblock
*/

void __load_superblock(void) {
    __read_sector();
}