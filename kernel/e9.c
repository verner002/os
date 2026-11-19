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
#include <kernel/devfs.h>

#define PORT            0xe9

static ssize_t e9_open(struct inode *inode, struct file *file) {
    if ((file->flags & O_ACCMODE) != O_WRONLY)
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
    return -EACCES;
}

static struct file_ops const e9_fops = {
    .open = &e9_open,
    .read = &e9_read,
    .write = &e9_write
};

int e9_init(void) {
    uint8_t value = inb(PORT);

    if (value != PORT)
        return -ENODEV;

    return devfs_register_chrdev("e9", &e9_fops);
}