/**
 * @file kdev.h
 * @author verner002
 * @date 02/09/2025
*/

#pragma once

#include "types.h"

typedef uint16_t kdev_t;

#define NO_DEV 0x0000

#define FLOPPY_MAJOR 0x02
#define HARDDISK_MAJOR 0x03

#define MAJOR(x) (((x) >> 8) & 0xff)
#define MINOR(x) ((x) & 0xff)
#define MAJMIN(x, y) (((x) << 8) | (y))

kdev_t name2kdev(char const *name);
char const *kdev2name(kdev_t kdev);