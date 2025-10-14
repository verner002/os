/**
 * @file mount.c
 * @author verner002
 * @date 03/10/2025
*/

#include "kernel/mount.h"
#include "hal/vfs.h"
#include "kernel/task.h"

int32_t __mount(__kdev_t dev, char const *mpoint) {
    struct __dentry *mount = (struct __dentry *)kmalloc(sizeof(struct __dentry));

    if (!mount)
        return -1;

    struct __inode *inode = (struct __inode *)kmalloc(sizeof(struct __inode));

    if (!inode) {
        kfree(mount);
        return -1;
    }

    __dentry_init(mount);
    mount->name = mpoint;
    
    __inode_init(inode, mount);
    inode->i_gid = 0;
    inode->i_uid = 0;
    inode->i_mode = 0x80000000 | 0755;

    struct __dentry *root = __get_dentry();
    __dentry_add(mount, root);
    return 0;
}