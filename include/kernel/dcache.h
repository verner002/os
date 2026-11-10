/**
 * @file dcache.h
 * @author verner002
 * @date 14/10/2026
*/

#pragma once

#include <kernel/list.h>
#include <kernel/atomic.h>
#include <kernel/inode.h>

// early declarations
struct inode;

struct dentry_ops {
    struct dentry *(* create)(struct dentry *dentry, char const  *name);
    struct dentry *(* lookup)(struct dentry *dentry, char const *name);
};

struct dentry {
    char name[32];
    struct dentry *parent;
    atomic_t refs;
    struct inode *inode;
    struct dentry_ops *ops;
    struct list siblings;
    struct list *children;
};

struct dentry *alloc_dentry(struct dentry *parent, char const *name);
struct dentry *dget(struct dentry *dentry);
void dput(struct dentry *dentry);
struct dentry *d_lookup(struct dentry *dentry, char const *name);
void instantiate(struct dentry *dentry, struct inode *inode);