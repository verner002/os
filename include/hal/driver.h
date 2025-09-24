/**
 * Driver
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "null.h"
#include "types.h"
#include "kernel/kobj.h"

/**
 * Types Definitions
*/

typedef struct __driver DRIVER;
typedef struct __symbol SYMBOL;

#define DRIVER_MOUNT_SYMBOL "mount"

/**
 * Enumeration
*/

enum __storage_class {
    SYMBOL_STORAGE_CLASS_EXT = 2,
    SYMBOL_STORAGE_CLASS_STAT = 3 
};

/**
 * Structures
*/

struct __driver {
    struct __kobj *d_kobj;
    int32_t (* release)(struct __driver * driver);
};

struct __attribute__((__packed__)) __symbol {
    uint8_t name[32];
    uint32_t address;
};

/**
 * Declarations
*/

int32_t __init_drivers(void);
struct __driver *__register_driver(char const *name, struct __kobj_type *ktype);