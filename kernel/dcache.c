/**
 * @file dentry.c
 * @author verner002
 * @date 14/10/2026
*/

#include <stdalign.h>
#include <string.h>

#include <kernel/dcache.h>
#include <kernel/slab.h>

// 1024 dentries (lru strategy)
#define DCACHE_BITS         10
#define DCACHE_SIZE         (1 << DCACHE_BITS)
#define DCACHE_LRU_LIMIT    1024

struct dentry *dcache_table[DCACHE_SIZE];

static uint32_t dcache_hash(struct dentry *parent, char const *name) {
    // fowler-noll-vo-1a hash function
    uint32_t hash = 2166136261u;
    uint32_t ptr = (uint32_t)parent;

    for (size_t i = 0; i < sizeof(ptr); ++i) {
        hash ^= ptr & 0xff;
        hash *= 16777619u;
        ptr >>= 8;
    }

    while (*name) {
        hash ^= *name++;
        hash *= 16777619u;
    }

    return hash;
}

static void dcache_insert(struct dentry *dentry) {
    uint32_t hash = dcache_hash(dentry->parent, dentry->name);
    uint32_t i = hash & (DCACHE_BITS - 1);
    
    dentry->hash_next = dcache_table[i];
    dcache_table[i] = dentry;
}

static struct dentry *dcache_lookup(struct dentry *parent, char const *name) {
    uint32_t hash = dcache_hash(parent, name);
    uint32_t i = hash & (DCACHE_BITS - 1);

    struct dentry *current = dcache_table[i];

    while (current) {
        if (current->parent == parent && !strcmp(current->name, name))
            return current;

        current = current->hash_next;
    }

    // cache miss
    return NULL;
}

struct kmem_cache *dentry_cache;

static size_t nr_dentries_unused;
static struct list_head dentry_unused;

void dcache_init(void) {
    dentry_cache = kmem_cache_create("dentry_cache", sizeof(struct dentry), alignof(struct dentry), NULL);
    nr_dentries_unused = 0;
    list_init(&dentry_unused);
}

struct dentry *alloc_dentry(struct dentry *parent, char const *name) {
    struct dentry *dentry = dcache_lookup(parent, name);
    
    if (dentry) {
        // cache hit!
        dget(dentry);
        list_link(dentry->l_unused.prev, dentry->l_unused.next);
        list_init(&dentry->l_unused);
        return dentry;
    }

    dentry = (struct dentry *)kmem_cache_alloc(dentry_cache, KM_NOSLEEP);

    if (!dentry)
        return NULL;

    memset(dentry, 0, sizeof(dentry));
    strncpy(dentry->name, name, sizeof(dentry->name) - 1);
    dentry->parent = parent;
    dentry->refs = 1;
    dentry->inode = NULL;
    dentry->ops = NULL;
    list_init(&(dentry->siblings));
    list_init(&(dentry->children));
    list_init(&dentry->l_unused);

    if (parent)
        list_add(&(parent->children), &(dentry->siblings));

    dcache_insert(dentry);
    return dentry;
}

/**
 * dget returns a reference to the given dentry
 * 
 * since we are running on single core cpu and
 * the kernel is non-preemtive, there are no risks
 * related to race conditions. the function increments
 * reference counter (the dentry is guaranteed to
 * stay in memory until reference counter reaches zero)
*/

struct dentry *dget(struct dentry *dentry) {
    if (!dentry)
        ++(dentry->refs);

    return dentry;
}

void dput(struct dentry *dentry) {
    if (--(dentry->refs) > 0)
        return;

    if (nr_dentries_unused >= DCACHE_LRU_LIMIT) {
        struct list_head *oldest = dentry_unused.prev;
        list_link(oldest->prev, oldest->next);
        
        if (dentry->inode)
            iput(dentry->inode);
        
        kmem_cache_free(dentry_cache, dentry);
    }

    list_push(&dentry_unused, &dentry->l_unused);
}

struct dentry *d_lookup(struct dentry *dentry, char const *name) {
    if (!dentry || !name)
        return NULL;
    
    for (struct list_head *list = dentry->children.next; list != &(dentry->children); list = list->next) {
        struct dentry *d = containerof(list, struct dentry, siblings);

        if (!strcmp(d->name, name))
            return dget(d);
    }

    return NULL;
}

void d_instantiate(struct dentry *dentry, struct inode *inode) {
    // ownership is transfered to dentry
    dentry->inode = inode;
}