/**
 * @file dev.c
 * @author verner002
 * @date 14/09/2025
*/

#include "macros.h"
#include "hal/dev.h"
#include "mm/heap.h"
#include "kernel/task.h"
#include "fs/file.h"
#include "fs/fat12/super.h"

atomic_t __dev_id;

// kobj for /dev
static struct dentry *dev;

// devs linked-list
static uint32_t devices_cnt;
static struct __dev *devices[32];

// RFC: use linked-lists?
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
    dev = create_file(current_dentry(), "dev", 0, 0, 0x80000000);

    if (!dev)
        return -1;

    devices_cnt = 0;

    spin_lock(&lock);
    devs_count = 0;
    spin_unlock(&lock);
    // TODO: init array(?)
    return 0;
}

/**
 * __dev_add
 * 
 * RFC: actually we should take a look at device types
 *  and (maybe) create different __*_dev_add functions
 *  for  each type?
*/

int32_t __dev_add(__kdev_t kdev, char const *name, struct __dev *parent, struct __bus *bus, struct __dev_type *type) {
    if (!name /*|| !bus || !type*/ || devices_cnt >= sizeofarray(devices))
        return -1;

    struct __dev *device = (struct __dev *)kmalloc(sizeof(struct __dev));

    if (!device)
        return -1;

    // FIXME: we need an atomic increment that
    // gives us the value and increments it as
    // a single operation
    device->d_id = __dev_id++;
    device->d_bus = bus;
    device->d_kdev = kdev;
    //device->d_kobj = ;
    device->d_name = name;
    device->d_parent = parent;
    device->d_type = type;

    struct dentry *dev_file = create_file(dev, name, 0, 0, 0x40000000 | 0755);

    if (!dev_file) {
        kfree(device);
        return -1;
    }

    devices[devices_cnt++] = device;
    return 0;
}

/*struct __dev *__dev_add(__kdev_t kdev, char const *name, struct __dev *parent, struct __dev_type *dev_type) {
    struct __dev *__dev = (struct __dev *)kmalloc(sizeof(struct __dev));

    if (!__dev)
        return NULL;

    struct __kobj *dev_kobj = (struct __kobj *)kmalloc(sizeof(struct __kobj));

    if (!dev_kobj) {
        kfree(__dev);
        return NULL;
    }

    struct __kobj_type *dev_kobj_type = (struct __kobj_type *)kmalloc(sizeof(struct __kobj_type));
    
    if (!dev_kobj_type) {
        kfree(dev_kobj);
        kfree(__dev);
        return NULL;
    }
    
    dev_kobj_type->k_attribs = NULL;
    dev_kobj_type->k_ops = (struct __sysfs_ops){
        .read = NULL,
        .write = NULL
    };
    dev_kobj_type->release = NULL;

    struct __dentry *dev_dentry = (struct __dentry *)kmalloc(sizeof(struct __dentry));
    
    if (!dev_dentry) {
        kfree(dev_kobj_type);
        kfree(dev_kobj);
        kfree(__dev);
        return NULL;
    }

    struct __inode *dev_inode = (struct __inode *)kmalloc(sizeof(struct __inode));

    if (!dev_inode) {
        kfree(dev_dentry);
        kfree(dev_kobj_type);
        kfree(dev_kobj);
        kfree(__dev);
        return NULL;
    }

    __inode_init(dev_inode, dev_dentry);
    dev_inode->i_gid = 0;
    dev_inode->i_uid = 0;
    dev_inode->i_mode = 0x80000000 | 0755;

    __dentry_init(dev_dentry);
    dev_dentry->name = name;
    dev_dentry->d_inode = dev_inode;
    
    struct __kobj parent_kobj = parent ? parent->d_kobj : ;

    if (!parent)
        parent = dev;

    __dentry_add(dev_dentry, parent->d_kobj->k_dentry);
    __kobj_init(dev_kobj, dev_kobj_type);

    dev_kobj->k_dentry = dev_dentry;
    __kobj_rename(dev_kobj, name);
    __kobj_add(dev_kobj, parent->d_kobj);

    __dev->d_kdev = kdev;
    __dev->d_kobj = dev_kobj;
    __dev->d_name = name;
    __dev->d_parent = parent;
    __dev->d_type = dev_type;

    __dev->d_id = __dev_id;
    atomic_inc(__dev_id);
    return __dev;
}*/

int register_blk_device(kdev_t kdev) {
    struct blk_device *device = (struct blk_device *)kmalloc(sizeof(struct blk_device));

    if (!device)
        return -1; // failed to allocate memory for blk device

    // TODO: detect fs
    struct super_block *super = (struct super_block *)kmalloc(sizeof(struct super_block));

    if (!super) {
        kfree(device);
        return -2; // failed to allocate memory for super block
    }

    int result = get_super(kdev, super);

    if (result)
        return result; // failed to read super

    device->h.dev = kdev;
    device->h.super = super;
    device->h.device_data = NULL;

    struct dentry *dev_file = create_file(dev, kdev2name(kdev), 0, 0, 0x40000000 | 0755);
    dev_file->inode->data = (void *)device;

    spin_lock(&lock);
    devs[devs_count++] = (struct device *)device;
    spin_unlock(&lock);
    return 0;
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