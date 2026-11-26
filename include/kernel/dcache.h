/**
 * @file dcache.h
 * @author verner002
 * @date 14/10/2026
*/

#pragma once

#include <stddef.h>

#include <kernel/list.h>

// forward declarations
struct inode;

struct dentry_ops {
    struct dentry *(* create)(struct dentry *dentry, char const  *name);
    struct dentry *(* lookup)(struct dentry *dentry, char const *name);
};

struct dentry {
    char name[32];
    struct dentry *parent;
    size_t refs;
    struct inode *inode;
    struct dentry_ops *ops;
    struct list_head siblings;
    struct list_head children;
    struct dentry *hash_next;
    struct list_head l_unused;
};

void dcache_init(void);
struct dentry *alloc_dentry(struct dentry *parent, char const *name);
struct dentry *dget(struct dentry *dentry);
void dput(struct dentry *dentry);
struct dentry *d_lookup(struct dentry *dentry, char const *name);
void d_instantiate(struct dentry *dentry, struct inode *inode);