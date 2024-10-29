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

/**
 * Types Definitions
*/

typedef struct _device DEVICE;

/**
 * Structures
*/

struct _device {
    byte id;
    char *name;
    void *driver;
    DEVICE **devices;
};