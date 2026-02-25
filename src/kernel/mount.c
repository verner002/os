/**
 * @file mount.c
 * @author verner002
 * @date 03/10/2025
*/

#include "kernel/mount.h"
#include "hal/dev.h"
#include "kernel/task.h"

#include "fs/fat12/fat12.h"

/**
 * mount
*/

int mount(kdev_t kdev, char const *mountpoint) {
    char *temp = (char *)kmalloc(strlen(mountpoint) + sizeof(char));

    if (!temp) {
        printf("failed to allocate memory for mountpoint string\n");
        return -1;
    }

    strcpy(temp, mountpoint);

    struct dentry *root = current_dentry();

    struct dentry *mountdir = root->d_ops->lookup(root, temp);

    kfree(temp);

    if (!mountdir) {
        printf("%s not found\n", mountpoint);
        return -1; // mountdir not found
    }

    // detect fs from superblock and then
    // decide which fs driver to use
    int result = __fat12_list_root(mountdir, kdev);

    return result;

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