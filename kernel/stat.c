/**
 * @file stat.c
 * @author verner002
 * @date 13/10/2026
*/

#include <errno.h>

#include <kernel/stat.h>
#include <kernel/process.h>

int sys_fstat(int fd, struct stat *st) {
    struct file *f;

    // short-circuit evaluation
    if (fd >= NFDESC || !(f = proctab[cproc].fdescs[fd]))
        return -ENOENT;

    struct inode *inode = f->dentry->inode;

    st->s_ino = inode->ino;
    st->s_uid = inode->uid;
    st->s_gid = inode->gid;
    st->s_mode = inode->mode;
    st->s_dev = inode->kdev;
    st->s_size = inode->size;

    return 0;
}