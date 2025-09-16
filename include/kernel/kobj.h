/**
 * @file kobj.h
 * @author verner002
 * @date 14/09/2025
*/

#pragma once

#include "types.h"
#include "hal/vfs.h"

struct  __sysfs_attrib;
struct __kobj;
struct __kobj_type;

struct __sysfs_ops {
    uint32_t (*read)(struct __kobj *kobj, struct __sysfs_attrib *attrib, char *buffer);
    void (*write)(struct __kobj *kobj, struct __sysfs_attrib *attrib, char const *buffer, uint32_t size);
};

struct  __sysfs_attrib {
    char *name;
};

struct __kobj_type {
    void (*release)(struct __kobj *);
    struct __sysfs_ops k_ops;
    struct __sysfs_attrib **k_attribs;
};

struct __kobj {
    char const *k_name;
    char k_sname[10]; // short name
    atomic_t k_refs;
    struct __kobj *k_parent;
    struct __kobj *k_previous;
    struct __kobj *k_next;
    struct __kobj_type *k_type;
    struct __dentry *k_dentry;
    // RFC: add mutex?
};

void __kobj_init(struct __kobj *kobj, struct __kobj_type *kobj_type);
void __kobj_add(struct __kobj *kobj, struct __kobj *parent);
void __kobj_rename(struct __kobj *kobj, char const *name);
void __kobj_release(struct __kobj *kobj);
void __kobj_put(struct __kobj *kobj);