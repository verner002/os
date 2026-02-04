/**
 * Driver
 * 
 * Author: verner002
*/

#pragma once

#include "null.h"
#include "types.h"
#include "kernel/kobj.h"

struct __driver {
    struct __kobj *kobj;
    struct __driver_type *type;
    // function to release the specific driver
    // structure (device driver for example)
    int32_t (* release)(struct __driver * driver);
};

// block device driver
struct __block_dev_driver {
    struct __driver h;
    // block offset, block count and buffer
    int32_t (* read)(uint32_t offset, uint32_t count, char *buffer);
    // buffer and block count
    int32_t (* write)(char const *buffer, uint32_t count);
    // command and data
    int32_t (* ioctl)(uint8_t cmd, void *data);
};

int32_t __init_drivers(void);
struct __driver *__register_driver(char const *name, uint8_t major, struct __kobj_type *ktype);
struct __driver *__driver_lookup(uint8_t major);