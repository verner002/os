/**
 * @file paging.c
 * @author verner002
 * @date 19/09/2026
*/

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <kernel/paging.h>
#include <kernel/types.h>
#include <kernel/system.h>
#include <kernel/process.h>
#include <kernel/pfa.h>

int kmap(void *dir, void *virt, void *phys, int flags) {
    uint32_t *pd = (uint32_t *)((uint32_t)dir + KERNEL_VIRT_BASE);
    uint32_t pd_idx = (uint32_t)virt >> 22;
    uint32_t pt_idx = ((uint32_t)virt >> 12) & 0x3ffu;

    uint32_t *pde = pd + pd_idx;
    uint32_t pt_phys = *pde & ~0xfffu;

    if (!(*pde & PAGE_P)) {
        pt_phys = (uint32_t)pfa_alloc();

        if (!pt_phys)
            return -ENOMEM;

        uint32_t *pt_new = (uint32_t *)(pt_phys + KERNEL_VIRT_BASE);
        memset(pt_new, 0, PAGE_SIZE);

        *pde = pt_phys | (flags & PAGE_US) | PAGE_RW | PAGE_P;
    } else if (flags & PAGE_US)
        *pde |= PAGE_US;

    uint32_t *pt = (uint32_t *)(pt_phys + KERNEL_VIRT_BASE);
    pt[pt_idx] = ((uint32_t)phys & ~0xfffu) | (uint32_t)flags | PAGE_P;
    
    /*asm volatile (
        "invlpg dword ptr [%0]"
        :
        : "r" (virt)
        : "memory"
    ); // invalidate tlb*/

    return 0;
}

static uint32_t vmm_get_pte(uint32_t *dir, size_t virt) {
    uint32_t pde = dir[virt >> 22];

    // no page table present
    if (!(pde & PAGE_P))
        return 0;

    uint32_t *pt = (uint32_t *)((pde & ~0xfffu) + KERNEL_VIRT_BASE);
    
    uint32_t pte = pt[((uint32_t)virt >> 12) & 0x3ffu];

    if (!(pde & PAGE_US))
        pte &= ~PAGE_US;

    return pte;
}

#define VAL_WRITE           1

static int validate_user_range(void const *addr, size_t size, int flags) {
    if (!size)
        return 1;

    size_t start_page = (size_t)addr & ~0xfffu;
    size_t end_page = ((size_t)addr + (size_t)size - 1) & ~0xfffu;
    uint32_t *pd = (uint32_t *)((current->cr3 & ~0xfffu) + KERNEL_VIRT_BASE);

    for (size_t page = start_page; page <= end_page; page += PAGE_SIZE) {
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