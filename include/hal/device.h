/**
 * @file device.h
 * @author verner002
 * @date 19/08/2025
*/

#pragma once

#include "types.h"
#include "kernel/atomic.h"
#include "kernel/kdev.h"
#include "hal/bus.h"

#define DEV_CHAR 0
#define DEV_BLK 1

struct char_device_ops {

};

struct blk_device_ops {

};

struct device;

// RFC: use static list to define if device is
//  char or block? would be better to explictly
//  specify the type 
struct device_type {
    char const *name;
    // frees memory of given device (diff types)
    int (* release)(struct device *device);
};

struct device {
    kdev_t dev;
    struct device_type *type;
    struct super_block *super;
    void *device_data;
};

struct blk_device {
    struct device h;
};

int32_t __dev_init(void);
int register_blk_device(kdev_t kdev);
struct device *lookup_device(kdev_t kdev);
int get_devs(struct device ***devs, int *count);