/**
 * @file bus.c
 * @author verner002
 * @date 15/09/2025
*/

#include "macros.h"
#include "hal/bus.h"
#include "hal/vfs.h"
#include "kernel/kobj.h"
#include "kernel/heap.h"

extern struct __dentry *sysfs;

uint32_t buses_cnt = 0;
struct __bus *buses[16];

struct __bus *__register_bus(char const *name, struct __driver const *driver) {
    if (buses_cnt >= sizeofarray(buses))
        return NULL;

    struct __inode *inode = __new_inode();
    struct __dentry *dentry = __new_dentry();
    // TODO: use __kobj_init
    struct __kobj *kobj = (struct __kobj *)kmalloc(sizeof(struct __kobj));
    struct __bus *bus = (struct __bus *)kmalloc(sizeof(struct __bus));

    if (!inode || !dentry || !kobj || !bus)
        return NULL;

    inode->i_dentry = dentry;
    //inode->i_refs = 1;
    
    dentry->d_parent = sysfs;
    dentry->d_inode = inode;
    //dentry->d_refs = 1;
    dentry->name = name;
    dentry->d_next = dentry->d_child;
    dentry->d_prev = NULL;
    dentry->d_child->d_prev = dentry;
    dentry->d_child = dentry;

    kobj->k_dentry = dentry;
    kobj->k_name = name;
    
    bus->b_kobj = kobj;
    bus->b_driver = driver;
    bus->b_name = name;

    buses[buses_cnt++] = bus;
    return bus;
}