/**
 * @file sysfs.h
 * @author verner002
 * @date 19/09/2025
*/

#pragma once

#include "types.h"
#include "kernel/kobj.h"

int32_t __init_sysfs(struct __dentry *mpoint);
void __sysfs_group_init(struct __kobj *kobj, char const *name);
void __sysfs_group_add(struct __kobj *kobj);