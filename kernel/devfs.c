/**
 * @file devfs.c
 * @author verner002
 * @date 19/11/2026
*/

#include <errno.h>

#include <kernel/devfs.h>
#include <kernel/dcache.h>

extern struct dentry *root_dentry;

struct dentry *devfs_dentry;

static struct dentry *devfs_lookup(struct dentry *dentry, char const *name) {
    return d_lookup(dentry, name);
}

int devfs_init(void) {
    if (!root_dentry)
        return -ENOENT;

    struct inode *devfs_inode = alloc_inode(0, 0, S_IFDIR | 0755);

    if (!devfs_inode)
        return -ENOMEM;

    devfs_dentry = alloc_dentry(root_dentry, "dev");

    if (!devfs_dentry) {
        iput(devfs_inode);
        return -ENOMEM;
    }

    d_instantiate(devfs_dentry, devfs_inode);
    return 0;
}

int devfs_register_chrdev(char const *name, struct file_ops const *fops) {
    if (!name || !fops)
        return -EINVAL;

    if (!devfs_dentry)
        return -ENOENT;

    struct dentry *found = devfs_lookup(devfs_dentry, name);

    if (found) {
        dput(found);
        return -EEXIST;
    }

    struct inode *dev_inode = alloc_inode(0, 0, S_IFCHR | 0666);

    if (!dev_inode)
        return -ENOMEM;

    dev_inode->fops = fops;

    struct dentry *dev_dentry = alloc_dentry(devfs_dentry, name);

    if (!dev_dentry) {
        iput(dev_inode);
        return -ENOMEM;
    }

    d_instantiate(dev_dentry, dev_inode);
    return 0;
}