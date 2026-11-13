/**
 * @file inode.c
 * @author verner002
 * @date 23/02/2026
*/

#include <stdalign.h>
#include <stddef.h>
#include <string.h>

#include <kernel/inode.h>
#include <kernel/slab.h>

struct kmem_cache *inode_cache;

ino_t ino_seq = 1;

void inode_ctor(void *obj) {
    memset(obj, 0, sizeof(struct inode));
}

void inode_init(void) {
    inode_cache = kmem_cache_create("inode_cache", sizeof(struct inode), alignof(struct inode), &inode_ctor);
}

struct inode *iget(uid_t uid, gid_t gid, mode_t mode) {
    struct inode *inode = (struct inode *)kmem_cache_alloc(inode_cache, KM_NOSLEEP);

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

    kmem_cache_free(inode_cache, inode);
}