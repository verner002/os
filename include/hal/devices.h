/**
 * Devices
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "null.h"
#include "types.h"
#include "kstdlib/errno.h"
#include "drivers/cpu.h"
#include "hal/driver.h"
#include "kernel/heap.h"

/**
 * Types Definitions
*/

typedef struct __device DEVICE;
typedef struct __device_node DEVICE_NODE;

/**
 * Structures
*/

struct __device {
    char const *name;
    DEVICE *parent;
    DRIVER *driver;
};

struct __device_node {
    char const *name;
    DEVICE *device;
    uint32_t subdevices_count;
    uint32_t subdevices_capacity;
    DEVICE_NODE **subdevices;
};