/**
 * File System
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "hal/filesystem.h"

/**
 * Static Global Variables
*/

static bool fsm_mutex = FALSE;
static uint32_t mount_points_count;
static uint32_t mount_points_capacity;
static MOUNT_POINT *mount_points;

/**
 * __init_fsm
*/

int32_t __init_fsm(void) {
    mount_points_count = 0;
    mount_points_capacity = MOUNT_POINT_MOUNTED;

    mount_points = (MOUNT_POINT *)malloc(sizeof(MOUNT_POINT) * mount_points_capacity);

    if (!mount_points) {
        printk("Failed to initialize FSM\n");
        return -1;
    }

    memset(mount_points, 0, sizeof(MOUNT_POINT) * mount_points_capacity);

    return 0;
}

/**
 * __mount
*/

int32_t __mount(DEVICE *dev, char const *path) {
    __mutex_lock(fsm_mutex);

    if (mount_points_count >= mount_points_capacity) {
        printk("Cannot mount device, no free slots\n");
        return -1;
    }

    mount_points[mount_points_count++] = (MOUNT_POINT) {
        .dev = dev,
        .path = path,
        .flags = MOUNT_POINT_MOUNTED,
        .mount_points = NULL
    };

    __mutex_unlock(fsm_mutex);
    return 0;
}

/**
 * __unmount
*/

int32_t __unmount(char const *path) {
    __mutex_lock(fsm_mutex);

    __mutex_unlock(fsm_mutex);
    return 0;
}