/**
 * @file devices.h
 * @author verner002
 * @date 19/08/2025
*/

#pragma once

#include "null.h"
#include "types.h"
#include "kstdlib/errno.h"
#include "drivers/cpu.h"
#include "hal/driver.h"
#include "kernel/heap.h"

typedef struct __device DEVICE;
typedef struct __device_node DEVICE_NODE;

struct __device {
    char const *name;
    uint32_t major; // device type (driver to use)
    uint32_t minor; // device id under specific type (driver)
    DEVICE *parent;
    DRIVER *driver;
};

struct __device_node {
    char const *name;
    DEVICE *device;
    uint32_t subdevices_count;
    uint32_t subdevices_capacity;
    DEVICE_NODE **subdevices;
};

int32_t __init_devman(void);
DEVICE *__find_device(DEVICE *devs, uint32_t devs_count, char const *name);
int32_t __add_subdevice(DEVICE_NODE *parent, DEVICE_NODE *child);