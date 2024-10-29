/**
 * Devices
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "hal/devices.h"

/**
 * Static Global Variables
*/

DEVICE root = {
    .id = 0,
    .driver = NULL,
};

/**
 * __init_devs
*/

void __init_devs(void) {
    // look for device to initialize
    // create a node with status EINIT
    // try to initialize it
    // change status to value in errno
    // repeate until all devices are listed
}