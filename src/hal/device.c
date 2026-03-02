/**
 * @file dev.c
 * @author verner002
 * @date 14/09/2025
*/

#include "macros.h"
#include "hal/device.h"
#include "mm/heap.h"
#include "kernel/task.h"
#include "fs/file.h"
#include "fs/fat12/super.h"

// /dev directory
static struct dentry *dev;

// RFC: use linked-list?
static spinlock_t lock = {
    .locked = false
};
static int devs_count;
static struct device *devs[256];

/**
 * initializes devs linked-list and
 * mounts /dev
*/

int32_t __dev_init(void) {
    dev = create_file(current_dentry(), "dev", 0, 0, S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    if (!dev)
        return -1;

    spin_lock(&lock);
    devs_count = 0;
    spin_unlock(&lock);
    // TODO: init array(?)
    return 0;
}

int register_blk_device(kdev_t kdev) {
    struct blk_device *device = (struct blk_device *)kmalloc(sizeof(struct blk_device));

    if (!device)
        return -1; // failed to allocate memory for blk device

    struct device_type *type = (struct device_type *)kmalloc(sizeof(struct device_type));

    if (!type) {
        kfree(device);
        return -2;
    }

    // TODO: detect fs
    struct super_block *super = (struct super_block *)kmalloc(sizeof(struct super_block));

    if (!super) {
        kfree(type);
        kfree(device);
        return -3; // failed to allocate memory for super block
    }

    int result = get_super(kdev, super);

    if (result)
        return result; // failed to read super

    char const *name = kdev2name(kdev);

    if (!name)
        name = "unk";

    type->name = name;
    type->release = NULL;

    device->h.dev = kdev;
    device->h.super = super;
    device->h.device_data = NULL;
    device->h.type = type;

    struct dentry *dev_file = create_file(dev, name, 0, 0, S_IFBLK | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    dev_file->inode->kdev = kdev;
    dev_file->inode->i_blkdev = device;
    dev_file->inode->super_block = device->h.super;

    spin_lock(&lock);
    devs[devs_count++] = (struct device *)device;
    spin_unlock(&lock);
    return 0;
}

struct device *lookup_device(kdev_t kdev) {
    for (int i = 0; i < devs_count; ++i)
        if (devs[i]->dev == kdev)
            return devs[i];

    return NULL;
}

int get_devs(struct device ***list, int *count) {
    spin_lock(&lock);
    struct device **devices = (struct device **)kmalloc(sizeof(struct device *) * devs_count);

    if (!devices) {
        *count = 0; // not necessary but...
        spin_unlock(&lock);
        return -1; // failed to allocate memory for devices list
    }

    // copy the list
    for (int i = 0; i < devs_count; ++i)
        devices[i] = devs[i];

    // pass list to user
    *list = devices;
    *count = devs_count;
    spin_unlock(&lock);
    return 0;
}