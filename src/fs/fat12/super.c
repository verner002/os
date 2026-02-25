/**
 * @file super.c
 * @author verner002
 * @date 04/11/2025
*/

#include "fs/super.h"
#include "fs/fat12/super.h"
#include "hal/driver.h"
#include "mm/heap.h"
#include "kernel/spinlock.h"

/**
 * reads super block from the device
 * 
 * TODO: use cache (flags)?
*/

#include "kstdlib/stdio.h"

int get_super(kdev_t kdev, struct super_block *super) {
    if (!super)
        return -1;

    // lookup device in dev list and use
    // associated blk driver's functions to
    // read the super block
    struct __block_dev_driver *driver = (struct __block_dev_driver *)driver_lookup(MAJOR(kdev));

    if (!driver)
        return -1; // no driver loaded

    struct fat12_info *info = (struct fat12_info *)kmalloc(sizeof(struct fat12_info));

    if (!info)
        return -1;
    
    int result = driver->read(MINOR(kdev), 0, 1, (char *)info);

    if (result)
        return result;

    spin_lock(&super->lock);
    super->dev = kdev;
    super->type = NULL; // TODO: set fs_type_fat12
    super->block_size = (uint32_t)info->bytes_per_sector;
    super->flags = 0;
    super->mount_point = NULL;
    super->fs_data = (void *)info;
    spin_unlock(&super->lock);
    return 0;
}

/**
 * writes super block to the device
*/

int32_t put_super(__kdev_t dev, struct super_block *super) {
    return -1;
}