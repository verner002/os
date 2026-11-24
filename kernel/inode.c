/**
 * @file inode.c
 * @author verner002
 * @date 23/02/2026
*/

#include <stdalign.h>
#include <stddef.h>

#include <kernel/inode.h>
#include <kernel/slab.h>

struct kmem_cache *inode_cache;

ino_t ino_seq = 1;

void inode_ctor(void *obj) {
    //struct inode *inode = (struct inode *)obj;

    // TODO: init semaphores
}

void inode_init(void) {
    inode_cache = kmem_cache_create("inode_cache", sizeof(struct inode), alignof(struct inode), &inode_ctor);
}

void inode_read_lock(struct inode *inode) {
    sem_wait(&inode->turnstile);

    if (++inode->readers == 1)
        sem_wait(&inode->rw_lock);
    
    sem_signal(&inode->turnstile);
}

void inode_read_unlock(struct inode *inode) {
    if (--inode->readers == 0)
        sem_signal(&inode->rw_lock);
}

void inode_write_lock(struct inode *inode) {
    sem_wait(&inode->turnstile);
    sem_wait(&inode->rw_lock);
}

void inode_write_unlock(struct inode *inode) {
    sem_signal(&inode->rw_lock);
    sem_signal(&inode->turnstile);
}

struct inode *alloc_inode(uid_t uid, gid_t gid, mode_t mode) {
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

struct inode *iget(struct inode *inode) {
    if (!inode)
        ++(inode->refs);

    return inode;
}

void iput(struct inode *inode) {
    if (--(inode->refs) > 0)
        return;

    // TODO: move to unused list, keep inode number for a while
    kmem_cache_free(inode_cache, inode);
}