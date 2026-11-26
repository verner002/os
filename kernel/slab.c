/**
 * @file slab.c
 * @author verner002
 * @date 20/10/2026
*/

#include <stddef.h>
#include <stdalign.h>
#include <errno.h>

#include <kernel/slab.h>
#include <kernel/pfa.h>
#include <kernel/system.h>
//#include <kernel/process.h>

// forward declarations
struct kmem_cache;
struct kmem_slab;

#define slabof(x)           ((struct kmem_slab *)((size_t)(x) & ~4095))
#define alignup(x, a)       (((x) + (a) - 1) & ~((a) - 1))

struct kmem_slab {
    struct kmem_cache *cache;
    struct kmem_slab *prev;
    struct kmem_slab *next;
    void *free;
    size_t slots_in_use;
    size_t total_slots;
};

static struct kmem_cache {
    char const *name;
    size_t size;
    size_t align;
    void (*ctor)(void *obj);

    size_t link_off;
    size_t stride;
    size_t first_obj_off;
    size_t objs_per_slab;

    size_t nempty;
    struct kmem_slab *empty;
    struct kmem_slab *partial;
    struct kmem_slab *full;
} kmem_cache;

static void list_push(struct kmem_slab **list, struct kmem_slab *item) {
    item->prev = NULL;
    item->next = *list;

    if (*list)
        (*list)->prev = item;

    *list = item;
}

static void list_remove(struct kmem_slab **list, struct kmem_slab *item) {
    if (item->prev)
        item->prev->next = item->next;
    else
        *list = item->next;

    if (item->next)
        item->next->prev = item->prev;

    item->prev = NULL;
    item->next = NULL;
}

static void obj_link_set(struct kmem_cache *cache, void *obj, void *value) {
    *(void **)(obj + cache->link_off) = value;
}

static void *obj_link_get(struct kmem_cache *cache, void *obj) {
    return *(void **)(obj + cache->link_off);
}

static int kmem_cache_setup(struct kmem_cache *cache, char const *name, size_t size, size_t align, void (*ctor)(void *obj)) {
    if (!align)
        align = sizeof(void *);

    if (align & (align - 1))
        return -EINVAL;

    if (size < sizeof(void *))
        size = sizeof(void *);

    size_t link_off;
    size_t raw;

    if (ctor) {
        link_off = alignup(size, align);;
        cache->link_off = link_off;
        raw = link_off + sizeof(void *);
    } else {
        link_off = 0;
        raw = size;
    }

    size_t stride = alignup(raw, align);
    size_t first_obj_off = alignup(sizeof(struct kmem_slab), align);

    if (first_obj_off + stride > 4096)
        return -EINVAL;

    size_t objs_per_slab = (4096 - first_obj_off) / stride;

    cache->name = name;
    cache->size = size;
    cache->align = align;
    cache->ctor = ctor;
    cache->link_off = link_off;
    cache->stride = stride;
    cache->first_obj_off = first_obj_off;
    cache->objs_per_slab = objs_per_slab;
    cache->nempty = 0;
    cache->empty = NULL;
    cache->partial = NULL;
    cache->full = NULL;

    return 0;
}

static struct kmem_slab *kmem_slab_alloc(struct kmem_cache *cache) {
    void *page = pfa_alloc() + KERNEL_VIRT_BASE;

    if (!page)
        return NULL;

    struct kmem_slab *slab = (struct kmem_slab *)page;
    
    slab->cache = cache;
    slab->slots_in_use = 0;
    slab->prev = NULL;
    slab->next = NULL;

    void *base = (void *)page + cache->first_obj_off;
    size_t stride = cache->stride;
    void (*ctor)(void *obj) = cache->ctor;

    size_t i = cache->objs_per_slab;
    void *head = NULL;

    while (i-- > 0) {
        void *obj = base + stride * i;

        if (ctor)
            ctor(obj);

        obj_link_set(cache, obj, head);

        head = obj;
    }

    slab->free = head;
    return slab;
}

void kmem_cache_init(void) {
    kmem_cache_setup(&kmem_cache, "kmem_cache", sizeof(struct kmem_cache), alignof(struct kmem_cache), NULL);
}

struct kmem_cache *kmem_cache_create(char const *name, size_t size, size_t align, void (*ctor)(void *obj)) {
    struct kmem_cache *cache = kmem_cache_alloc(&kmem_cache, KM_NOSLEEP);

    if (!cache) {
        //errno = ENOMEM; -- kmem_cache_alloc sets errno
        return NULL;
    }

    if (kmem_cache_setup(cache, name, size, align, ctor)) {
        kmem_cache_free(&kmem_cache, cache);
        errno = EINVAL; // set to kmem_cache_setup return value?
        return NULL;
    }

    return cache;
}

void *kmem_cache_alloc(struct kmem_cache *cache, int flags) {
    if (flags == KM_SLEEP) {
        errno = ENOSYS;
        return NULL;
    }

    struct kmem_slab *slab;

    if (cache->partial) {
        slab = cache->partial;
    } else if (cache->empty) {
        slab = cache->empty;

        --cache->nempty;
        list_remove(&cache->empty, slab);
        list_push(&cache->partial, slab);
    } else {
        slab = kmem_slab_alloc(cache);

        if (!slab) {
            errno = ENOMEM;
            return NULL;
        }

        list_push(&cache->partial, slab);
    }

    void *obj = slab->free;
    slab->free = obj_link_get(cache, obj);
    ++slab->slots_in_use;

    if (!slab->free) {
        list_remove(&cache->partial, slab);
        list_push(&cache->full, slab);
    }

    return obj;
}

void kmem_cache_free(struct kmem_cache *cache, void *obj) {
    if (!obj)
        return;
    
    struct kmem_slab *slab = slabof(obj);

    int was_full = !slab->free;

    obj_link_set(cache, obj, slab->free);
    slab->free = obj;
    --slab->slots_in_use;

    if (was_full) {
        list_remove(&cache->full, slab);
        list_push(&cache->partial, slab);
    }

    if (!slab->slots_in_use) {
        list_remove(&cache->partial, slab);

        if (!cache->empty) {
            list_push(&cache->empty, slab);
            ++cache->nempty;
        } else
            pfa_dealloc((void *)((size_t)slab - KERNEL_VIRT_BASE));
    }
}

void kmem_cache_destroy(struct kmem_cache *cache) {
    struct kmem_slab *slab = cache->empty;

    while (slab) {
        struct kmem_slab *next = slab->next;
        pfa_dealloc((void *)((size_t)next - KERNEL_VIRT_BASE));
        slab = next;
    }

    cache->empty = NULL;
    kmem_cache_free(&kmem_cache, (void *)cache);
}