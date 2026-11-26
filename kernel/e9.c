/**
 * @file e9.c
 * @author verner002
 * @date 19/11/2026
*/

#include <stddef.h>
#include <limits.h>
#include <errno.h>

#include <kernel/io.h>
#include <kernel/inode.h>
#include <kernel/fcntl.h>
#include <kernel/device.h>

#define E9_DEV          MAJMIN(240, 0)
#define PORT            0xe9

static struct cdev e9_cdev;
static struct device *e9_device;

static ssize_t e9_open(struct inode *inode, struct file *file) {
    if ((file->flags & O_ACCMODE) != O_RDWR)
        return -EACCES;

    return 0;
}

static ssize_t e9_write(struct file *file, char const *buffer, size_t count) {    
    if (count > SSIZE_MAX)
        return -EINVAL;
    
    for (ssize_t i = 0; i < (ssize_t)count; ++i)
        outb(PORT, buffer[i]);

    return count;
}

static ssize_t e9_read(struct file *file, char *buffer, size_t count) {
    if (count > SSIZE_MAX)
        return -EINVAL;
    
    for (ssize_t i = 0; i < (ssize_t)count; ++i)
        buffer[i] = inb(PORT);

    return count;
}

static struct file_ops const e9_fops = {
    .open = &e9_open,
    .read = &e9_read,
    .write = &e9_write
};

int e9_init(void) {
    int err;

    uint8_t value = inb(PORT);

    if (value != PORT)
        return -ENODEV;

    cdev_init(&e9_cdev, &e9_fops);
    
    err = cdev_add(&e9_cdev, E9_DEV, 1);

    if (err < 0)
        return err;

    e9_device = device_create(E9_DEV, "e9");

    if (!e9_device) {
        cdev_remove(&e9_cdev);
        return -ENOMEM;
    }

    return 0;
}