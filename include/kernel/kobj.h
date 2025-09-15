/**
 * @file kobj.h
 * @author verner002
 * @date 14/09/2025
*/

#pragma once

#include "types.h"
#include "hal/vfs.h"

struct __kobj_type {
    void (*k_release)(struct __kobj *);
};

struct __kobj {
    char const *k_name;
    char k_sname[10]; // short name
    uint32_t k_refs;
    struct __kobj *k_previous;
    struct __kobj *k_next;
    struct __kobj *k_parent;
    struct __kobj_type *k_type;
    struct __dentry *k_dentry;
};