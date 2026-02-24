/**
 * @file sysfs.h
 * @author verner002
 * @date 19/09/2025
*/

#pragma once

#include "types.h"
#include "fs/dentry.h"

extern struct dentry *sysfs;

int sysfs_init(struct dentry *root);
int sysfs_register_group(char const *name);