/**
 * @file mount.c
 * @author verner002
 * @date 03/10/2025
*/

#include "kernel/atomic.h"
#include "kernel/mount.h"
#include "hal/device.h"
#include "kernel/task.h"
#include "fs/file.h"

#include "fs/fat12/fat12.h"
#include "fs/fat12/super.h"

atomic_t mount_points_count = 0;

struct mount_point {
    kdev_t kdev;
    char const *path;
} mount_points[256];

/**
 * mount
*/

int mount(char const *device, char const *mountpoint) {
    // TODO: device = abspath(device) - free after use!

    errno = 0;
    struct dentry *dev_file = lookup_file(device);

    if (errno == ENOENT || errno == ENOTDIR || !(dev_file->inode->mode & S_IFBLK) || device[strlen(device) - 1] == '/') {
        printf("mount: cannot lookup blkdev '%s'\n", device);
        return -1;
    }

    if (dev_file->inode->super_block->mount_point) {
        printf("mount: device already mounted\n");
        return -2;
    }

    char *temp = (char *)kmalloc(strlen(mountpoint) + sizeof(char));

    if (!temp) {
        printf("failed to allocate memory for mountpoint string\n");
        return -3;
    }

    strcpy(temp, mountpoint);

    struct dentry *root = current_dentry();

    struct dentry *mountdir = root->d_ops->lookup(root, temp);

    kfree(temp);

    if (!mountdir) {
        printf("mount: %s not found\n", mountpoint);
        return -4; // mountdir not found
    }

    spin_lock(&dev_file->inode->super_block->lock);
    dev_file->inode->super_block->mount_point = mountdir->inode;
    spin_unlock(&dev_file->inode->super_block->lock);

    /*if (get_super(dev_file->inode->kdev, dev_file->inode->super_block)) {
        printf("mount: failed to read super block\n");
        return -4;   
    }*/

    // detect fs from superblock and then
    // decide which fs driver to use
    int result = __fat12_list_root(mountdir, dev_file->inode->kdev);

    if (result)
        return result;

    mount_points[atomic_fetch_add(&mount_points_count, 1)] = (struct mount_point){
        .kdev = dev_file->inode->kdev,
        .path = mountpoint
    };

    // "/mnt/test/fd0", "./test/fd0", "test/fd0", "test/fd0/"
    // '.' (dot) represent current directory relative
    // to the given part of the path or (if used as
    // the first char the relative address to the
    // current working directory - process start
    // directory)
    //
    // RFC: do we want to perform a mount operation
    //  to the existing dentry? could be useful for
    //  example when trying to mount a temp disk image
    //  from an external device (and using the known
    //  one if that is not possible)
    //   - i'll keep this open and use the easier
    //     approach (not allow that)
    return 0;
}

char *lookup_mountpoint(kdev_t kdev) {
    for (int i = 0; i < mount_points_count; ++i) {
        struct mount_point *mp = &mount_points[i];

        if (mp->kdev == kdev)
            return mp->path;
    }

    return NULL;
}