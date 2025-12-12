/**
 * @file sysfs.c
 * @author verner002
 * @date 19/09/2025
*/

#include "kernel/sysfs.h"
#include "mm/heap.h"
#include "kstdlib/stdio.h"
#include "hal/vfs.h"

struct __kobj *sysfs;
static uint32_t groups_cnt = 0;
// TODO: implement kernel objects set
static struct __kobj  *groups[16];

int32_t __sysfs_init(struct __dentry *mpoint) {
    sysfs = (struct __kobj *)kmalloc(sizeof(struct __kobj));
    
    if (!sysfs) {
        printk("failed to allocate memory for sysfs\n");
        return -1;
    }

    struct __dentry *sys = (struct __dentry *)kmalloc(sizeof(struct __dentry));

    __dentry_init(sys);
    sys->name = "sys";
    __dentry_add(sys, mpoint);

    if (!sys) {
        kfree(sysfs);
        printk("failed to create dentry \"sys\"\n");
        return -1;
    }

    struct __inode *sys_mdata = (struct __inode *)kmalloc(sizeof(struct __inode));

    if (!sys_mdata) {
        kfree(sys);
        kfree(sysfs);
        printk("failed to kmalloc memory for sysfs inode\n");
        return -1;
    }

    __inode_init(sys_mdata, sys);

    sys->d_inode = sys_mdata; // assign metadata

    *sysfs = (struct __kobj){
        .k_name = NULL,
        .k_sname = { "sysfs\0\0\0\0" },
        .k_refs = 1,
        .k_type = NULL,
        .k_parent = NULL,
        .k_previous = NULL,
        .k_next = NULL,
        .k_dentry = sys
    };

    return 0;
}

/**
 * initializes the kobj so it is a part of
 * sysfs (mapped in vfs in /sys)
*/

void __sysfs_group_init(struct __kobj *kobj, char const *name) {
    atomic_inc(sysfs->k_refs);
    kobj->k_parent = sysfs;
}

void __sysfs_group_add(struct __kobj *kobj) {

}