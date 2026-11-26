/**
 * @file vfs.c
 * @author verner002
 * @date 15/10/2026
*/

#include <stdalign.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <kernel/vfs.h>
#include <kernel/inode.h>
#include <kernel/fcntl.h>
#include <kernel/slab.h>
#include <kernel/process.h>

struct dentry *root_dentry;

static struct kmem_cache *file_cache;

void vfs_init(void) {
    struct inode *inode = alloc_inode(0, 0, S_IFDIR | 0755);
    root_dentry = alloc_dentry(NULL, "/");
    d_instantiate(root_dentry, inode);
    root_dentry->parent = root_dentry;

    file_cache = kmem_cache_create("file_cache", sizeof(struct file), alignof(struct file), NULL);
}

struct dentry *walk_to_parent(char const *path, char last[NAME_MAX + 1], int *trailing_slash) {
    struct dentry *cd;

    char const *p = path;

    if (!*p) {
        errno = ENOENT;
        return NULL;
    }

    if (*p == '/') {
        while (*++p == '/');
        cd = root_dentry;
    } else
        cd = current->cwd;

    dget(cd);

    while (1) {
        size_t length = strcspn(p, "/");
        char const *next = p + length;

        if (trailing_slash)
            *trailing_slash = *next == '/';

        while (*next == '/')
            ++next;

        // last component
        if (!*next) {
            if (!(cd->inode) || !S_ISDIR(cd->inode->mode)) {
                dput(cd);
                errno = ENOTDIR;
                return NULL;
            }

            if (last) {
                memcpy(last, p, length);
                last[length] = '\0';
            }

            if (cd == root_dentry)
                *trailing_slash = 1;

            return cd;
        }

        if (!(cd->inode) || !S_ISDIR(cd->inode->mode)) {
            dput(cd);
            errno = ENOTDIR;
            return NULL;
        }

        struct dentry *nd;

        if (length == 1 && *p == '.')
            nd = dget(cd);
        else if (length == 2 && *p == '.' && *(p + 1) == '.')
            nd = dget(cd->parent);
        else {
            char tmp[NAME_MAX + 1];
            memcpy(tmp, p, length);
            tmp[length] = '\0';

            nd = d_lookup(cd, tmp);

            if (!nd) {
                dput(cd);
                errno = ENOENT;
                return NULL;
            }
        }

        dput(cd);
        cd = nd;
        p = next;
    }
}

struct file *vfs_open(char const *path, int flags, int mode) {
    char last[NAME_MAX + 1];
    int isdir;
    struct dentry *parent = walk_to_parent(path, last, &isdir);

    if (!parent)
        return NULL;

    struct dentry *dentry = d_lookup(parent, last);

    if (!dentry) {
        if (!(flags & O_CREAT)) {
            errno = ENOENT;
            return NULL;
        }

        dentry = alloc_dentry(parent, last);

        if (!dentry) {
            errno = EAGAIN;
            dput(dentry);
            return NULL;
        }
    } else {
        if (isdir && !S_ISDIR(dentry->inode->mode)) {
            errno = ENOTDIR;
            return NULL;
        }

        if ((flags & O_CREAT) && (flags & O_EXCL)) {
            errno = EEXIST;
            dput(dentry);
            return NULL;
        }

        if (flags & O_TRUNC) {
            // TODO: truncate, check mode?
        }
    }

    dput(parent);
    
    // TODO: check if allocation didn't fail
    struct file *file = kmem_cache_alloc(file_cache, KM_NOSLEEP);
    file->count = 1;
    file->dentry = dentry;
    file->fops = dentry->inode->fops;
    file->flags = flags;

    if (file->fops && file->fops->open) {
        ssize_t err = file->fops->open(dentry->inode, file);

        if (err < 0) {
            dput(file->dentry);
            kmem_cache_free(file_cache, file);
            return NULL;
        }
    }

    return file;
}

int vfs_close(struct file *file) {
    if (--(file->count) > 0)
        return 0;
        
    // TODO: call fops->release
    dput(file->dentry);
    kmem_cache_free(file_cache, file);
    return 0;
}