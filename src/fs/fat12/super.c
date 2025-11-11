/**
 * @file super.c
 * @author verner002
 * @date 04/11/2025
*/

#include "fs/fat12/super.h"
#include "hal/dev.h"

/**
 * reads super block from the device
*/

int32_t get_super(__kdev_t dev, struct __superblock *super) {
    // lookup device in dev list and use
    // associated driver's functions to
    // read the super block
}

/**
 * writes super block to the device
*/

int32_t put_super(__kdev_t dev, struct __superblock *super) {

}