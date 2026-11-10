/**
 * @file inode.c
 * @author verner002
 * @date 23/02/2026
*/

#include <stddef.h>

#include <kernel/inode.h>
#include <kernel/kmalloc.h>

ino_t ino_seq = 1;

struct inode *iget(uid_t uid, gid_t gid, mode_t mode) {
    struct inode *inode = (struct inode *)kzalloc(sizeof(struct inode));

    if (!inode)
        return NULL;

    inode->ino = ino_seq++;
    inode->sb = NULL;
    inode->uid = uid;
    inode->gid = gid;
    inode->mode = mode;
    inode->refs = 1;
    inode->size = 0;
    inode->ops = NULL;
    inode->fops = NULL;
    return inode;
}

void iput(struct inode *inode) {
    if (--(inode->refs) > 0)
        return;

    kfree(inode);
}