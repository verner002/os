/**
 * @file inode.c
 * @author verner002
 * @date 23/02/2026
*/

#include "null.h"
#include "fs/inode.h"
#include "mm/heap.h"

struct inode *get_inode(uid_t uid, gid_t gid, mode_t mode) {
    struct inode *inode = (struct inode *)kmalloc(sizeof(struct inode));

    if (!inode)
        return NULL;

    inode->super_block = NULL;
    inode->uid = uid;
    inode->gid = gid;
    inode->mode = mode;
    inode->size = 0;
    inode->i_ops = NULL;
    inode->refs = 1;
    inode->child = NULL;
    return inode;
}