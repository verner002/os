/**
 * @file dev.h
 * @author verner002
 * @date 19/08/2025
*/

#pragma once

#include "types.h"
#include "kernel/kdev.h"
#include "kernel/kobj.h"
#include "hal/bus.h"

struct __dev_type {
    // device attributes
    void (* d_release)(struct __device *);
};

struct __dev {
    char const *d_name;
    atomic_t d_id;
    __kdev_t d_kdev;
    struct __dev *d_parent;
    struct __kobj *d_kobj;
    struct __dev_type *d_type;
    struct __bus *d_bus;
};

extern atomic_t __dev_id;

int32_t __dev_init(void);
struct __dev *__dev_add(__kdev_t kdev, char const *name, struct __dev *parent, struct __dev_type *dev_type);