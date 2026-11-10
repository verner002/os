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

struct file_ops {
    int (*open)(int fd, char const *mode);
    //int (*seek)(int fd, int offset, int mode);
    int (*read)(int fd, char *buffer, int offset, int count);
    int (*write)(int fd, char const *buffer, int count);
    int (*close)(int fd);
    // TODO: add dir operations
};

struct device;

// each device is identified by its
// kdev number (major:minor)
// all devices define file operations
// no matter whether it is char or block
struct device {
    kdev_t kdev;
    void *device_data;
    struct file_ops f_ops;
};

struct char_device {
    struct device h;
};

// super block is specific for block devs
struct blk_device {
    struct device h;
    // reference to super block?
    unsigned long block_size;       // block size dev works with
};

int32_t __dev_init(void);
int register_blk_dev(kdev_t kdev);
struct device *lookup_device(kdev_t kdev);
int get_devs(struct device ***devs, int *count);