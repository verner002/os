/**
 * @file inode.h
 * @author verner002
 * @date 20/02/2026
*/

#pragma once

#include "kernel/atomic.h"
#include "kernel/user.h"
#include "fs/super.h"

typedef int mode_t;

struct inode_ops {

};

struct inode {
    mode_t mode; // permissions & mode
    uid_t uid; // user id
    gid_t gid; // group id
    atomic_t refs; // reference counter

    struct dentry *child;

    uint32_t size; // size in bytes
    
    struct super_block *super_block;
    struct inode_ops *i_ops;

    void *data;
};

/*struct file_ops {
    int (* open)();
    int (* read)();
    int (* lseek)();
    int (* write)();
    int (* readdir)();
    int (* release)();
};*/

struct inode *get_inode(uid_t uid, gid_t gid, mode_t mode);