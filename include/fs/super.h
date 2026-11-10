/**
 * @file super.h
 * @author verner002
 * @date 11/09/2025
*/

#pragma once

#include "types.h"
#include "bool.h"
#include "kernel/kdev.h"
#include "kernel/spinlock.h"

#define FS_READ_ONLY 0x80000000

struct fs_type {
    union {
        //struct fat12_data
        //struct ext1_data
        //...
    };
};

struct super_block {
    kdev_t dev;
    unsigned long block_size;        // block size fs works with
    uint32_t flags;             // flags
    spinlock_t lock;            // TODO: replace with rwlock_t
    struct fs_type *type;
    struct inode *mount_point;
    void *fs_data;              // fs specific data
};