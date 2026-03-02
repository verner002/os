/**
 * @file file.c
 * @author verner002
 * @date 23/02/2026
*/

#include "fs/file.h"
#include "mm/heap.h"
#include "kernel/task.h"

extern struct dentry root_dentry;


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

struct dentry *create_file(struct dentry *parent, char const *name, uid_t uid, gid_t gid, mode_t mode) {
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

    return dentry;
}

/**
 * stat
*/

int stat(char const *path) {
    struct dentry *file = lookup_file(path);

    return file->inode->mode;
}