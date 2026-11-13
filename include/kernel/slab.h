/**
 * @file slab.h
 * @author verner002
 * @date 12/11/2026
*/

#pragma once

#define KM_SLEEP            1 // may sleep
#define KM_NOSLEEP          2 // will not sleep

struct kmem_cache;

void kmem_cache_init(void);
struct kmem_cache *kmem_cache_create(char const *name, size_t size, size_t align, void (*ctor)(void *obj));
void *kmem_cache_alloc(struct kmem_cache *cache, int flags);
void kmem_cache_free(struct kmem_cache *cache, void *obj);
void kmem_cache_destroy(struct kmem_cache *cache);