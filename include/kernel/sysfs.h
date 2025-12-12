/**
 * @file sysfs.h
 * @author verner002
 * @date 19/09/2025
*/

#pragma once

#include "types.h"
#include "kernel/kobj.h"

extern struct __kobj *sysfs;

int32_t __sysfs_init(struct __dentry *mpoint);
void __sysfs_group_init(struct __kobj *kobj, char const *name);
void __sysfs_group_add(struct __kobj *kobj);