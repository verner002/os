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
    
};

struct super_block {
    kdev_t dev;
    uint32_t block_size;
    uint32_t flags; // flags
    spinlock_t lock; // TODO: replace with rwlock_t
    struct fs_type *type;
    struct inode *mount_point;
    void *fs_data; // fs specific data
};