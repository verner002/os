/**
 * @file devfs.c
 * @author verner002
 * @date 19/11/2026
*/

#pragma once

#include <kernel/inode.h>

int devfs_init(void);
int devfs_register_chrdev(char const *name, struct file_ops const *fops);