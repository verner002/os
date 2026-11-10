/**
 * @file super.c
 * @author verner002
 * @date 07/10/2026
*/

#include <kernel/kdev.h>
#include <kernel/inode.h>
#include <kernel/fat12.h>

struct fs_type {
    union {
        struct fat12_data;
    };
};

struct super_block {
    kdev_t kdev;
    unsigned long flags;
    struct fs_type *fs_type;
    struct inode *mount_point;
    void *fs_data;
};