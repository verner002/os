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
#include "kstdlib/stdlib.h"

/**
 * Types Definitions
*/

typedef struct _device DEVICE;

/**
 * Structures
*/

struct __device {
    char const *name;
    DEVICE *parent;
};