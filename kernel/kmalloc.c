/**
 * @file kmalloc.c
 * @author verner002
 * @date 09/10/2026
*/

#include <string.h>

#include <kernel/kmalloc.h>

#define CHUNK_MAGIC     0x0badc0de
#define CHUNK_FREE      1

struct chunk {
    unsigned long magic;    
    unsigned char flags;
    size_t size;
    struct chunk *next;
    struct chunk *previous;
};

struct chunk
    *kheap = NULL,
    *kheap_offset = NULL;

void kheap_init(void *start, size_t size) {
    kheap = start;
    kheap->magic = CHUNK_MAGIC;
    kheap->flags = CHUNK_FREE;
    kheap->size = size - sizeof(struct chunk);
    kheap->next = NULL;
    kheap->previous = NULL;
    kheap_offset = kheap;
}

void *kmalloc(size_t size) {
    if (!size)
        return NULL;

    struct chunk *current = kheap_offset;

    size_t const real_size = sizeof(struct chunk) + size;

    while (current) {
        if (current->flags & CHUNK_FREE && current->size >= real_size) {
            struct chunk *next;
            size_t remainder = current->size - real_size;
            
            if (remainder > sizeof(struct chunk)) {
                next = (struct chunk *)((char *)current + real_size);
                next->magic = CHUNK_MAGIC;
                next->flags = CHUNK_FREE;
                next->size = remainder - sizeof(struct chunk);
                next->next = current->next;
                next->previous = current;

                if (current->next)
                    current->next->previous = next;

                current->next = next;
                current->size = size;
            } else
                next = current->next;

            kheap_offset = next;
            current->flags &= ~CHUNK_FREE;

            return (void *)((char *)current + sizeof(struct chunk));
        }

        current = current->next;
    }

    kheap_offset = kheap;
    return NULL;
}

void *kzalloc(size_t size) {
    void *ptr = kmalloc(size);

    if (!ptr)
        return NULL;

    return memset(ptr, 0, size);
}

static struct chunk *merge_neighbours(struct chunk *chunk) {
    struct chunk *previous = chunk->previous;

    if (previous && previous->flags & CHUNK_FREE) {
        previous->size += sizeof(struct chunk) + chunk->size;
        previous->next = chunk->next;

        if (chunk->next)
            chunk->next->previous = previous;

        chunk = previous;
    }

    struct chunk *next = chunk->next;

    if (next && next->flags & CHUNK_FREE) {
        chunk->size += sizeof(struct chunk) + chunk->size;
        chunk->next = next->next;

        if (next->next)
            next->next->previous = chunk;
    }

    return chunk;
}

void kfree(void *ptr) {
    if (!ptr)
        return;

    struct chunk *current = (struct chunk *)((char *)ptr - sizeof(struct chunk));

    if (current->magic != CHUNK_MAGIC || current->flags & CHUNK_FREE)
        return;

    current->flags |= CHUNK_FREE;
    current = merge_neighbours(current);

    if ((void *)current < (void *)kheap_offset)
        kheap_offset = current;
}