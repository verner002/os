/**
 * @file device.c
 * @author verner002
 * @date 26/11/2026
*/

#include <stdalign.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <kernel/device.h>
#include <kernel/slab.h>
#include <kernel/devfs.h>

struct kmem_cache *device_cache;

struct list_head cdev_list;

struct list_head devices_list;

void device_init(void) {
    device_cache = kmem_cache_create("device_cache", sizeof(struct device), alignof(struct device), NULL);
    list_init(&cdev_list);
    list_init(&devices_list);
}

void cdev_init(struct cdev *cdev, struct file_ops const *fops) {
    memset(cdev, 0, sizeof(struct cdev));
    list_init(&(cdev->list));
    cdev->ops = fops;
}

int cdev_add(struct cdev *cdev, kdev_t kdev, unsigned int count) {
    if (!cdev || !cdev->ops || !count)
        return -EINVAL;

    // check for overlap
    kdev_t new_start = kdev;
    kdev_t new_end = new_start + count - 1;

    for (struct list_head *item = cdev_list.next; item != &cdev_list; item = item->next) {
        struct cdev *cdev_item = containerof(item, struct cdev, list);

        kdev_t cdev_start = cdev_item->kdev;
        kdev_t cdev_end = cdev_start + cdev_item->count - 1;

        if (new_start <= cdev_end && new_end >= cdev_start)
            return -EBUSY;        
    }

    cdev->kdev = kdev;
    cdev->count = count;
    list_add(&cdev_list, &(cdev->list));
    return 0;
}

void cdev_remove(struct cdev *cdev) {
    if (!cdev)
        return;

    list_link(cdev->list.prev, cdev->list.next);
    list_init(&(cdev->list));
}

struct cdev *cdev_lookup(kdev_t dev) {
    for (struct list_head *item = cdev_list.next; item != &cdev_list; item = item->next) {
        struct cdev *cdev = containerof(item, struct cdev, list);

        if (dev >= cdev->kdev && dev < (cdev->kdev + cdev->count))
            return cdev;
    }

    return NULL;
}

struct device *device_create(kdev_t kdev, char const *name) {
    struct device *dev = kmem_cache_alloc(device_cache, KM_NOSLEEP);

    if (!dev)
        return NULL;

    memset(dev, 0, sizeof(struct device));
    list_init(&(dev->classmates));
    list_init(&(dev->siblings));
    list_init(&(dev->children));

    dev->kdev = kdev;
    memset(dev->name, 0, 16);
    strncpy(dev->name, name, 15);

    if (dev->parent)
        list_add(&(dev->parent->children), &(dev->siblings));
    else
        list_add(&devices_list, &(dev->siblings));

    if (dev->kdev != 0) {
        int err = devfs_register_chrdev(name, kdev, S_IFCHR);

        if (err < 0) {
            list_link(dev->siblings.prev, dev->siblings.next);
            list_init(&(dev->siblings));
            kmem_cache_free(device_cache, dev);
            return NULL;
        }
    }

    return dev;
}