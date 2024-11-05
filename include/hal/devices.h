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
#include "cpu.h"

#include "kstdlib/errno.h"
#include "kstdlib/stdlib.h"

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
    int status;
};