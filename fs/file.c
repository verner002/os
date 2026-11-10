/**
 * @file file.c
 * @author verner002
 * @date 23/02/2026
*/

#include "fs/file.h"
#include "mm/heap.h"
#include "kernel/task.h"

extern struct dentry root_dentry;

struct file {
    char *buffer; // buffer base ptr
    uint32_t size; // buffer size
    uint32_t count; // chars in buffer
    char *head; // buffer read ptr
    uint32_t index; // write index
    uint32_t flags; // flags
    mode_t mode; // mode and permissions
    spinlock_t lock; // file lock
};

/**
 * abspath
 * 
 * NOTE: converts relative path to a clean absolute path
 * EXAMPLE:
 *    /././///mn/photos/./image1.png -> /mnt/photos/image1.png
 *    ./eclk.pdf -> /home/verner002/documents/eclk.pdf
 *    ~/music/
*/

char *abspath(char const *path) {
    char *temp = (char *)kmalloc(strlen(path) + sizeof(char));

    if (!temp)
        return NULL;

    strcpy(temp, path);

    char *buffer = temp;
    char *token;

    char *abs;

    while ((token = strtok_r(NULL, "/", &buffer))) {

    }

    kfree(temp);
    return abs;
}

struct dentry *lookup_file(char const *path) {
    struct dentry *origin;

    if (*path == '/') {
        ++path;
        origin = &root_dentry;
    } else
        origin = current_dentry();

    return origin->d_ops->lookup(origin, path);
}

/**
 * create_file
 * 
 * @brief Creates file in the VFS structure under given
 *  dentry.
*/

struct dentry *create_file(struct dentry *parent, char const *name, uid_t uid, gid_t gid, mode_t mode, struct file_ops *f_ops) {
    struct inode *inode = get_inode(uid, gid, mode);

    if (!inode)
        return NULL; // failed to get inode

    int name_len = strlen(name);
    char *filename = (char *)kmalloc(name_len + sizeof(char));

    if (!filename) {
        kfree(inode);
        return NULL;
    }

    strcpy(filename, name);

    struct dentry *dentry = get_dentry(parent, filename, inode);

    if (!dentry) {
        kfree(filename);
        kfree(inode);
        return NULL; // failed to get dentry
    }

    if (mode & S_IFDIR) {
        char *dot_fname = (char *)kmalloc(2 * sizeof(char));

        strcpy(dot_fname, ".");

        // TODO: check for null
        struct dentry *dot = get_dentry(dentry, dot_fname, inode);

        char *dotdot_fname = (char *)kmalloc(3 * sizeof(char));

        strcpy(dotdot_fname, "..");

        // TODO: check for null
        struct dentry *dotdot = get_dentry(dentry, dotdot_fname, parent->inode);
    }

    inode->f_ops = f_ops;
    return dentry;
}

/**
 * stat
*/

mode_t stat(char const *path) {
    struct dentry *file = lookup_file(path);

    return file->inode->mode;
}

struct inode *files[256];
atomic_t fid_seq = 0;

FILE *fopen(char const *path, char const *mode) {
    errno = 0;
    struct dentry *dentry = lookup_file(path);

    FILE *f = (FILE *)kmalloc(sizeof(FILE));

    if (!f)
        return NULL;

    int fid = atomic_fetch_add(&fid_seq, 1);
    struct inode *inode = dentry->inode;

    files[fid] = inode;

    char *buffer = (char *)pgalloc(PAGE_MAP);
    int r = inode->f_ops->read(inode, buffer);
    f->__base = buffer;
    f->__count = inode->size;
    f->__index = 0;
    f->__ptr = buffer;
    f->__lock = false;
    f->__flags = 0;
    f->__ready = false;
    f->fid = fid;
}