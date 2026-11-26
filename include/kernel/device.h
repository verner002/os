/**
 * @file device.h
 * @author verner002
 * @date 26/11/2026
*/

#pragma once

#include <kernel/kdev.h>
#include <kernel/inode.h>
#include <kernel/list.h>

// forward declarations
struct bus_type;
struct device_class;
struct device_driver;

struct cdev {
    kdev_t kdev;
    unsigned int count;
    struct file_ops const *ops;
    struct list_head list;
};

struct device {
    char name[16];
    kdev_t kdev;

    struct device *parent;
    struct list_head classmates;
    struct list_head siblings;
    struct list_head children;

    struct bus_type *bus;
    struct device_class *class;
    struct device_driver *driver;

    void *driver_data;
    void (*release)(struct device *dev);
};

void device_init(void);
void cdev_init(struct cdev *cdev, struct file_ops const *fops);
int cdev_add(struct cdev *cdev, kdev_t kdev, unsigned int count);
void cdev_remove(struct cdev *cdev);
struct cdev *cdev_lookup(kdev_t dev);
struct device *device_create(kdev_t kdev, char const *name);