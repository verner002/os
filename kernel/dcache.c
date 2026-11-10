/**
 * @file dentry.c
 * @author verner002
 * @date 14/10/2026
*/

#include <string.h>

#include <kernel/dcache.h>
#include <kernel/kmalloc.h>

// 128 dentries (lru strategy)
#define DCACHE_SIZE         128

struct dentry *alloc_dentry(struct dentry *parent, char const *name) {
    struct dentry *dentry = (struct dentry *)kzalloc(sizeof(struct dentry));

    if (!dentry)
        return NULL;

    strncpy(dentry->name, name, sizeof(dentry->name) - 1);
    dentry->parent = parent;
    dentry->refs = 1;
    dentry->inode = NULL;
    dentry->ops = NULL;
    dentry->children = NULL;

    if (parent) {
        dentry->siblings.next = parent->children;
        parent->children = &dentry->siblings;
    }

    return dentry;
}

/**
 * dget returns a reference to the given dentry
 * 
 * since we are running on single core cpu and
 * the kernel is non-preemtive, there are no risks
 * related to race conditions. the function increments
 * reference counter (the dentry is guaranteed to
 * stay in memory until reference counter reaches zero)
*/

struct dentry *dget(struct dentry *dentry) {
    if (!dentry)
        atomic_increment(&dentry->refs);

    return dentry;
}

void dput(struct dentry *dentry) {

}

struct dentry *d_lookup(struct dentry *dentry, char const *name) {
    for (struct list *list = dentry->children; list; list = list->next) {
        struct dentry *d = containerof(list, struct dentry, children);

        if (!strcmp(d->name, name))
            return d;
    }

    return NULL;
}

void instantiate(struct dentry *dentry, struct inode *inode) {
    // ownership is transfered to dentry
    dentry->inode = inode;
}