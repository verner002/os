/**
 * @file dev.h
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

struct __dev_type {
    // device attributes
    void (* d_release)(struct __device *);
};

struct __dev {
    char const *d_name;
    atomic_t d_id;
    __kdev_t d_kdev;
    struct __dev *d_parent;
    struct __dev_type *d_type;
    struct __bus *d_bus;
};

extern atomic_t __dev_id;

int32_t __dev_init(void);
int32_t __dev_add(__kdev_t kdev, char const *name, struct __dev *parent, struct __bus *bus, struct __dev_type *type);
//struct __dev *__dev_add(__kdev_t kdev, char const *name, struct __dev *parent, struct __dev_type *dev_type);
int register_blk_device(kdev_t kdev);
int get_devs(struct device ***devs, int *count);