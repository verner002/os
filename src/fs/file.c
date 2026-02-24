/**
 * @file file.c
 * @author verner002
 * @date 23/02/2026
*/

#include "fs/file.h"
#include "mm/heap.h"

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

    if (mode & 0x80000000) {
        char *dot_fname = (char *)kmalloc(2 * sizeof(char));

        strcpy(dot_fname, ".");

        struct dentry *dot = get_dentry(dentry, dot_fname, inode);

        char *dotdot_fname = (char *)kmalloc(3 * sizeof(char));

        strcpy(dotdot_fname, "..");

        struct dentry *dotdot = get_dentry(dentry, dotdot_fname, parent->inode);
    }

    return dentry;
}