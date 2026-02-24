/**
 * @file bus.h
 * @author verner002
 * @date 14/09/2025
*/

#pragma once

#include "hal/driver.h"

struct __bus_type {

};

struct __bus {
    char const *b_name;
    struct __bus_type *b_type;
    struct __driver *b_driver;
};

int32_t __init_buses(void);
struct __bus *__register_bus(char const *name, struct __driver const *driver);