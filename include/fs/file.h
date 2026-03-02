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

struct dentry *lookup_file(char const *path);
struct dentry *create_file(struct dentry *parent, char const *name, uid_t uid, gid_t gid, mode_t mode);
int stat(char const *path);