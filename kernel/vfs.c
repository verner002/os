/**
 * @file vfs.c
 * @author verner002
 * @date 15/10/2026
*/

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <kernel/fcntl.h>
#include <kernel/dcache.h>
#include <kernel/process.h>

#define NAME_MAX        255
#define PATH_MAX        4095

static struct dentry *root;

void vfs_init(void) {
    struct inode *inode = iget(0, 0, S_IFDIR | 0755);
    root = alloc_dentry(NULL, "/");
    instantiate(root, inode);
    root->parent = root;
}

struct dentry *walk_to_parent(char const *path, char last[NAME_MAX + 1]) {
    struct dentry *cd;

    char const *p = path;

    if (!*p)
        return -ENOENT;

    if (*p == '/') {
        while (*++p == '/');
        cd = root;
    } else
        cd = proctab[cproc].cwd;

    dget(cd);

    while (1) {
        size_t length = strcspn(p, "/");
        char const *next = p + length;

        while (*next == '/')
            ++next;

        // last component
        if (!*next) {
            strncpy(last, p, length);
            last[length] = '\0';

            if (!(cd->inode) || !S_ISDIR(cd->inode->mode)) {
                dput(cd);
                return -ENOTDIR;
            }

            return cd;
        }

        if (!(cd->inode) || !S_ISDIR(cd->inode->mode)) {
            dput(cd);
            return -ENOTDIR;
        }

        struct dentry *nd;

        if (length == 1 && *p == '.')
            nd = dget(cd);
        else if (length == 2 && *p == '.' & *(p + 1) == '.')
            nd = dget(cd->parent);
        else {
            char tmp[NAME_MAX + 1];
            strncpy(tmp, p, length);
            tmp[length] = '\0';

            nd = d_lookup(cd, tmp);

            if (!nd) {
                dput(cd);
                return -ENOENT;
            }

            dget(nd);
        }

        dput(cd);
        cd = nd;
        p = next;
    }
}

struct file *vfs_open(char const *path, int flags, int mode) {

}