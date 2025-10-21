/**
 * @file mount.c
 * @author verner002
 * @date 03/10/2025
*/

#include "kernel/mount.h"
#include "hal/dev.h"
#include "kernel/task.h"

/**
 * __mount
*/

int32_t __mount(__kdev_t dev, char const *mpoint) {
    __dev_init();

    __file_add(__get_dentry()->io_ops.lookup(__get_dentry(), "/dev"), "fd0", 0, 0, 0x80000000 | 0755);
    
    // "/mnt/test/fd0", "./test/fd0", "test/fd0", "test/fd0/"
    // '.' (dot) represent current directory relative
    // to the given part of the path or (if used as
    // the first char the relative address to the
    // current working directory - process start
    // directory)
    //
    // RFC: do we want to perform a mount operation
    //  to the existing __dentry? could be useful for
    //  example when trying to mount a temp disk image
    //  from an external device (and using the known
    //  one if that is not possible)
    //   - i'll keep this open and use the easier
    //     approach (not allow that)

    // firstly let's find the node we want to
    // perform the mount operation to
    char *temp = (char *)kmalloc(strlen(mpoint) + sizeof(char));
    
    //if (!temp) error
    
    strcpy(temp, mpoint);

    struct __dentry *thread_mount = __get_dentry();
    struct __dentry *mount_dir = thread_mount->io_ops.lookup(thread_mount, temp);
    kfree(temp);

    if (!mount_dir) {
        printk("mount point \'%s\' not found\n", mpoint);
        return -1;
    }

    return 0;
}