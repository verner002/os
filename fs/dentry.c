/**
 * @file dentry.c
 * @author verner002
 * @date 21/02/2026
*/

#include "null.h"
#include "kstdlib/errno.h"
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
        dentry->next->previous = dentry;

    dentry->inode = inode;
    dentry->refs = 1;

    return dentry;
}

static struct dentry *internal_dentry_lookup(struct dentry *node, char **strtok_buffer) {
    /*char *token;

    if (*strtok_buffer == '/') {
        if (node != &root_dentry)
            return NULL;

        ++*strtok_buffer;
        return internal_dentry_lookup(&root_dentry, strtok_buffer);
    }*/
    
    char *token = strtok_r(NULL, "/", strtok_buffer);

    if (!token)
        return node;

    if (!(node->inode->mode & S_IFDIR)) {
        errno = ENOTDIR;
        return NULL;
    }

    struct dentry *child = node->inode->child;

    while (child) {
        if (!strcmp(token, child->name))
            return internal_dentry_lookup(child, strtok_buffer);

        child = child->next;
    }

    errno = ENOENT;
    return NULL;
}

struct dentry *dentry_lookup(struct dentry *node, char const *path) {
    // strtok is destructiv, make local copy
    char *temp = (char *)kmalloc(strlen(path) + sizeof(char));
    
    if (!temp) {
        errno = ENOMEM;
        return NULL;
    }

    // copy the path
    strcpy(temp, path);

    // local strtok buffer
    char *strtok_buffer = temp;
    struct dentry *result = internal_dentry_lookup(node, &strtok_buffer);

    kfree(temp);
    return result;
}