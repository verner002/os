/**
 * @file paging.c
 * @author verner002
 * @date 19/09/2026
*/

#include <stddef.h>
#include <string.h>

#include <kernel/paging.h>
#include <kernel/types.h>
#include <kernel/system.h>
#include <kernel/process.h>

__attribute__((aligned(4096))) char pool[4096];

int kmap(void *dir, void *virt, void *phys, int flags) {
    uint32_t *pd = (uint32_t *)((uint32_t)dir + KERNEL_VIRT_BASE);
    
    uint32_t *pde = &pd[(uint32_t)virt >> 22];

    uint32_t *pt = (uint32_t *)(*pde & ~4095);

    if (!pt) {
        pt = (uint32_t *)((uint32_t)pool - KERNEL_VIRT_BASE);
        *pde = (uint32_t)pt | PAGE_US | PAGE_RW | PAGE_P;
    }

    pt = (uint32_t *)((uint32_t)pt + KERNEL_VIRT_BASE);

    uint32_t *pte = &pt[((uint32_t)virt >> 12) & 0x3ff];

    *pte = ((uint32_t)phys & ~4095) | (uint32_t)flags;

    return 0;
}

static uint32_t vmm_get_pte(uint32_t *dir, size_t virt) {
    uint32_t pde = dir[virt >> 22];

    // no page table present
    if (!(pde & PAGE_P))
        return 0;

    uint32_t *pt = (uint32_t *)((pde & ~4095) + KERNEL_VIRT_BASE);
    
    uint32_t pte = pt[((uint32_t)virt >> 12) & 0x3ff];

    if (!(pde & PAGE_US))
        pte &= ~PAGE_US;

    return pte;
}

#define VAL_WRITE           1

static int validate_user_range(void *addr, size_t size, int flags) {
    if (!size)
        return 1;

    size_t start_page = (size_t)addr & ~4095;
    size_t end_page = ((size_t)addr + (size_t)size - 1) & ~4095;
    uint32_t *pd = (uint32_t *)((current->cr3 & ~4095) + KERNEL_VIRT_BASE);

    for (size_t page = start_page; page <= end_page; page += 4096) {
        uint32_t pte = vmm_get_pte(pd, page);

        if (!(pte & PAGE_P) || !(pte & PAGE_US))
            return 0;

        if ((flags & VAL_WRITE) && !(pte & PAGE_RW))
            return 0;
    }

    return 1;
}

size_t copy_from_user(void *dst, void const *src, size_t size) {
    if (!validate_user_range(src, size, 0))
        return 0;

    memcpy(dst, src, size);
    return size;
}

size_t copy_to_user(void *dst, void const *src, size_t size) {
    if (!validate_user_range(dst, size, 1))
        return 0;

    memcpy(dst, src, size);
    return size;
}