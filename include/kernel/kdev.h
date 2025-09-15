/**
 * @file kdev.h
 * @author verner002
 * @date 02/09/2025
*/

#pragma once

#include "types.h"
#include "kernel/super.h"

typedef uint16_t __kdev_t;

#define NO_DEV 0x0000
#define FLOPPY_MAJOR 0x02
#define HARDDISK_MAJOR 0x03

#define MAJOR(x) (((x) >> 8) & 0xff)
#define MINOR(x) ((x) & 0xff)

struct __kdev {
    __kdev_t k_dev;
    struct __superblock *k_super;
    struct __kdev *k_prev;
    struct __kdev *k_next;
};

uint16_t __dev_name_to_kdev(char const *dev_name);
int32_t __add_kdev(__kdev_t k_kdev, struct __superblock *super);