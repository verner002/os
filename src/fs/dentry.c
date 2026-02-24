/**
 * @file dentry.c
 * @author verner002
 * @date 21/02/2026
*/

#include "null.h"
#include "fs/dentry.h"
#include "mm/heap.h"

struct dentry *get_dentry(struct dentry *parent, char const *name, struct inode *inode) {
    struct dentry *dentry = (struct dentry *)kmalloc(sizeof(struct dentry));

    if (!dentry)
        return NULL;

    dentry->parent = parent;

    if (parent)
        atomic_increment(&parent->refs);

    dentry->name = name;
    dentry->inode = inode;
    dentry->d_ops = NULL;
    dentry->previous = NULL;
    dentry->next = parent->inode->child;
    parent->inode->child = dentry;

    if (dentry->next)
        dentry->previous = dentry;

    dentry->inode = inode;
    dentry->refs = 1;

    return dentry;
}