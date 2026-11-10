/**
 * @file file.h
 * @author verner002
 * @date 23/02/2026
*/

#pragma once

#include "fs/inode.h"
#include "fs/dentry.h"
#include "kernel/user.h"

/**
 * bit | meaning
 *   0 | others execute
 *   1 | other write
 *   2 | pther read
 *   3 | group execute
 *   4 | group write
 *   5 | group read
 *   7 | user execute
 *   8 | user write
 *   5 | user read
 *  32 | directory 
*/

/*struct file_ops {
    int (* open)();
    int (* read)();
    int (* lseek)();
    int (* write)();
    int (* readdir)();
    int (* release)();
};*/

struct dentry *lookup_file(char const *path);
struct dentry *create_file(struct dentry *parent, char const *name, uid_t uid, gid_t gid, mode_t mode, struct file_ops *f_ops);
mode_t stat(char const *path);

#include "kstdlib/file.h"
FILE *fopen(char const *path, char const *mode);