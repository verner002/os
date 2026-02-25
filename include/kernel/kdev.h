/**
 * @file kdev.h
 * @author verner002
 * @date 02/09/2025
*/

#pragma once

#include "types.h"

typedef uint16_t kdev_t;
typedef uint16_t __kdev_t;

#define NO_DEV 0x0000

#define FLOPPY_MAJOR 0x02
#define HARDDISK_MAJOR 0x03

#define MAJOR(x) (((x) >> 8) & 0xff)
#define MINOR(x) ((x) & 0xff)
#define MAJMIN(x, y) (((x) << 8) | (y))

/*struct __kdev {
    __kdev_t k_dev;
    struct super_block *k_super;
    struct __kdev *k_prev;
    struct __kdev *k_next;
};*/

uint16_t __dev_name_to_kdev(char const *dev_name);
char const *kdev2name(kdev_t kdev);
//int32_t __add_kdev(__kdev_t k_kdev, struct super_block *super);