/**
 * Driver
 * 
 * Author: verner002
*/

#pragma once

#include "null.h"
#include "types.h"

struct __driver {
    struct __driver_type *type;
    // function to release the specific driver
    // structure (device driver for example)
    int32_t (* release)(struct __driver * driver);
};

// block device driver
struct __block_dev_driver {
    struct __driver h;
    // block offset, block count and buffer
    int32_t (* read)(uint8_t minor, uint32_t offset, uint32_t count, char *buffer);
    // buffer and block count
    int32_t (* write)(uint8_t minor, char const *buffer, uint32_t count);
    // command and data
    int32_t (* ioctl)(uint8_t minor, uint8_t cmd, void *data);
};

int32_t drivers_init(void);
//struct __driver *__register_driver(char const *name, uint8_t major, struct __kobj_type *ktype);
int register_blk_driver(uint8_t major, int (*read)(uint8_t minor, uint32_t offset, uint32_t count, char *buffer));
struct __driver *driver_lookup(uint8_t major);