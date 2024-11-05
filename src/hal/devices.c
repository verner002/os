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
    .driver = NULL
};

/**
 * __init_devs
*/

void __init_devs(void) {
    root.devices = (DEVICE **)malloc(sizeof(DEVICE));

    DEVICE cpu = {
        .id = 1,
        .driver = NULL,
        .name = "Central Processing Unit", // replace with cpuid
        .status = EBUSY
    };

    root.devices[0] = &cpu;

    // init cpu
    cpu.status = 0; // __init_cpu()

    // look for device to initialize (use config file?)
    // create a node with status EBUSY
    // try to initialize it
    // change status to value in errno
    // repeate until all devices are listed
}