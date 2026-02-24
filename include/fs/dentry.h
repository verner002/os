/**
 * @file dentry.h
 * @author verner002
 * @date 21/02/2026
*/

#pragma once

#include "kernel/atomic.h"
#include "fs/inode.h"

struct dentry_ops {
    struct dentry *(* lookup)(struct dentry *dentry, char const *name);
};

struct dentry {
    char const *name;
    atomic_t refs;
    struct dentry *parent;
    struct dentry *previous;
    struct dentry *next;
    struct inode *inode;
    struct dentry_ops *d_ops;
};

struct dentry *get_dentry(struct dentry *parent, char const *name, struct inode *inode);