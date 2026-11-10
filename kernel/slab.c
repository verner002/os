/**
 * @file slab.c
 * @author verner002
 * @date 20/10/2026
*/

#include <stddef.h>

//#include <kernel/slab.h>
#include <kernel/pfa.h>
#include <kernel/process.h>

#define KM_SLEEP        1 // may sleep
#define KM_NOSLEEP      2 // will not sleep

//#define HEAD(x)         ((x) & ~4095)

// early declarations
struct kmem_cache;

struct kmem_slab {
    struct kmem_cache *cache;
    struct kmem_slab *prev;
    struct kmem_slab *next;
    void *free;
    size_t slots_in_use;
    size_t total_slots;
};

struct kmem_cache {
    char const *name;
    size_t size;
    size_t align;
    
    void (* ctor)(void *obj);
    void (* dtor)(void *obj);

    struct kmem_slab *empty;
    struct kmem_slab *partial;
    struct kmem_slab *full;
};

static struct kmem_cache kmem_cache;

void kmem_cache_init(void) {
    kmem_cache.name = "kmem_cache";
    kmem_cache.size = sizeof(struct kmem_cache);
    kmem_cache.align = 4096;
    kmem_cache.ctor = NULL;
    kmem_cache.dtor = NULL;
    kmem_cache.empty = NULL;
    kmem_cache.partial = NULL;
    kmem_cache.full = NULL;
}

struct kmem_cache *kmem_cache_create(char const *name, size_t size, size_t align, void *(* init)(void *obj)) {
    struct kmem_cache *cache = kmem_cache_alloc(&kmem_cache, KM_NOSLEEP);
}

void *kmem_cache_alloc(struct kmem_cache *cache, int flags) {
    struct kmem_slab *slab;

    if (kmem_cache.partial) {
        slab = kmem_cache.partial;
    } else if (kmem_cache.empty) {
        slab = kmem_cache.empty;
    } else {
        // request a page and create a new slab
        slab = pfa_alloc();
        
        if (!slab/* && (flags & KM_NOSLEEP)*/)
            return NULL;

        slab->cache = cache;
        slab->prev = NULL;
        slab->next = NULL;

        size_t head_size = ((sizeof(*cache) + cache->align - 1) / cache->align) * cache->align;

        slab->free = (void *)slab + head_size;
        slab->slots_in_use = 0;
        slab->total_slots = (4096 - head_size) / cache->size;

        // this slab is in fact empty
        // and at the end of the function
        // we move it to partial list
        kmem_cache.empty = slab;

        cache->empty = slab;
    }

    if (!slab->slots_in_use) {

    }
}

void kmem_cache_free(struct kmem_cache *cache, void *obj) {
    
}

void kmem_cache_destroy(struct kmem_cache *cache) {

}