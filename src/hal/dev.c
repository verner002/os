/**
 * @file dev.c
 * @author verner002
 * @date 14/09/2025
*/

#include "hal/dev.h"
#include "mm/heap.h"
#include "kernel/task.h"

atomic_t __dev_id;

static struct __kobj *dev;

static struct __dev *head = NULL;
static struct __dev *tail = NULL;

int32_t __dev_init(void) {
    dev = (struct __kobj *)kmalloc(sizeof(struct __kobj));

    if (!dev)
        return -1;

    struct __dentry *dev_dentry = (struct __dentry *)kmalloc(sizeof(struct __dentry));

    if (!dev_dentry) {
        kfree(dev);
        dev = NULL;
        return -1;
    }

    struct __inode *dev_inode = (struct __inode *)kmalloc(sizeof(struct __inode *));

    if (!dev_inode) {
        kfree(dev_dentry);
        kfree(dev);
        dev = NULL;
        return -1;
    }

    __dentry_init(dev_dentry);
    dev_dentry->name = "dev";
    dev_dentry->d_inode = dev_inode;
    __dentry_add(dev_dentry, __get_dentry());

    __inode_init(dev_inode, dev_dentry);
    dev_inode->i_mode = 0x80000000 | 0755;
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