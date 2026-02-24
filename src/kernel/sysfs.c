/**
 * @file sysfs.c
 * @author verner002
 * @date 19/09/2025
*/

#include "kernel/sysfs.h"
#include "mm/heap.h"
#include "kstdlib/stdio.h"
#include "fs/inode.h"

struct dentry *sysfs;

int sysfs_init(struct dentry *root) {
    struct inode *inode = get_inode(0, 0, 0x80000000 | 0555);

    if (!inode)
        return -1;

    sysfs = get_dentry(root, "sys", inode);

    if (!sysfs)
        return -2;

    return 0;
}

int sysfs_register_group(char const *name) {
    struct inode *inode = get_inode(0, 0, 0x80000000);

    if (!inode)
        return -1;

    struct dentry *group = get_dentry(sysfs, name, inode);

    if (!group)
        return -2;

    return 0;
}