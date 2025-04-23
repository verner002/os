/**
 * File System
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"
#include "hal/devices.h"
#include "kstdlib/stdio.h"
#include "kstdlib/stdlib.h"

/**
 * Types Definitions
*/

typedef struct __mount_point MOUNT_POINT;

/**
 * Constants
*/

#define MOUNT_POINTS_DEFAULT 8

#define MOUNT_POINT_MOUNTED 1
#define MOUNT_POINT_READ_ONLY 2

/**
 * Structures
*/

struct __mount_point {
    DEVICE *dev;
    char const *path;
    uint8_t flags;
    MOUNT_POINT *mount_points;
};

/**
 * Declarations
*/

int32_t __init_fsm(void);
int32_t __mount(DEVICE *dev, char const *path);
int32_t __unmount();