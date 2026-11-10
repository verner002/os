/**
 * @file paging.c
 * @author verner002
 * @date 19/09/2026
*/

#include <stddef.h>

#include <kernel/paging.h>
#include <kernel/types.h>
#include <kernel/system.h>

__attribute__((aligned(4096))) char pool[4096];

int kmap(void *dir, void *virt, void *phys, int flags) {
    uint32_t *pd = (uint32_t *)((uint32_t)dir + KERNEL_VIRT_BASE);
    
    uint32_t *pde = &pd[(uint32_t)virt >> 22];

    uint32_t *pt = (uint32_t *)(*pde & ~4095);

    if (!pt) {
        pt = (uint32_t *)((uint32_t)pool - KERNEL_VIRT_BASE);
        *pde = (uint32_t)pt | PAGE_CD | PAGE_WT | PAGE_US | PAGE_RW | PAGE_P;
    }

    pt = (uint32_t *)((uint32_t)pt + KERNEL_VIRT_BASE);

    uint32_t *pte = &pt[((uint32_t)virt >> 12) & 0x3ff];

    *pte = ((uint32_t)phys & ~4095) | (uint32_t)flags;

    return 0;
}