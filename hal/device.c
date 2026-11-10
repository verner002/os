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
static atomic_t devs_count = 0;
static struct device *devs[256];

/**
 * initializes devs linked-list and
 * mounts /dev
*/

int32_t __dev_init(void) {
    dev = create_file(current_dentry(), "dev", 0, 0, S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, NULL);

    // TODO: init array(?)
    return dev ? 0 : -1;
}

int register_char_dev(kdev_t kdev) {
    struct char_device *device = (struct char_device *)kmalloc(sizeof(struct char_device));

    if (!device)
        return -1;

    device->h.kdev = kdev;

    // TODO: add to /dev
    devs[atomic_fetch_add(&devs_count, 1)] = (struct device *)device;
    return 0;
}

int register_blk_dev(kdev_t kdev) {
    struct blk_device *device = (struct blk_device *)kmalloc(sizeof(struct blk_device));

    if (!device)
        return -1; // failed to allocate memory for blk device

    // TODO: detect fs
    struct super_block *super = (struct super_block *)kmalloc(sizeof(struct super_block));

    if (!super) {
        kfree(device);
        return -3; // failed to allocate memory for super block
    }

    int result = get_super(kdev, super);

    if (result)
        return result; // failed to read super

    char const *name = kdev2name(kdev);

    if (!name)
        name = "unk";

    device->h.kdev = kdev;
    device->super = super;
    device->h.device_data = NULL;

    struct dentry *dev_file = create_file(dev, name, 0, 0, S_IFBLK | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, NULL);
    dev_file->inode->kdev = kdev;
    dev_file->inode->i_blkdev = device;
    dev_file->inode->super_block = device->super;

    devs[atomic_fetch_add(devs_count, 1)] = (struct device *)device;
    return 0;
}

struct device *lookup_device(kdev_t kdev) {
    for (int i = 0; i < devs_count; ++i)
        if (devs[i]->kdev == kdev)
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