/**
 * @file fs.h
 * @author verner002
 * @date 03/09/2025
*/

#pragma once

#include "types.h"
#include "hal/vfs.h"

#define FS_READ_ONLY 1

struct __fs {
    uint16_t f_kdev;
    uint32_t f_block_size;
    uint32_t f_flags;
    struct __dentry f_mount;
};

struct __fs_ops {
    int32_t (* __mount)(struct __fs *);
};