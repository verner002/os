/**
 * @file kdev.h
 * @author verner002
 * @date 02/09/2025
*/

#pragma once

#include "types.h"

typedef uint16_t __kdev_t;

#define NO_DEV 0x0000
#define FLOPPY_MAJOR 0x02

#define MAJOR(x) (((x) >> 8) & 0xff)
#define MINOR(x) ((x) & 0xff)

struct __kdev {
    __kdev_t k_dev;
};

uint16_t __dev_name_to_kdev(char const *dev_name);