/**
 * @file vfs.h
 * @author verner002
 * @date 13/11/2026
*/

#pragma once

#include <kernel/dcache.h>
#include <kernel/fcntl.h>

#define NAME_MAX        255
#define PATH_MAX        4095

void vfs_init(void);
struct file *vfs_open(char const *path, int flags, int mode);
int vfs_close(struct file *file);