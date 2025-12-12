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
    struct __kobj *d_kobj;
    int32_t (* release)(struct __driver * driver);
    int32_t (* ioctl)(uint8_t cmd, void *data);
    // read_char/read_block?
};

int32_t __init_drivers(void);
struct __driver *__register_driver(char const *name, uint8_t major, struct __kobj_type *ktype);
struct __driver *__driver_lookup(uint8_t major);