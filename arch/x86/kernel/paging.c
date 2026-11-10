/**
 * @file paging.c
 * @author verner002
 * @date 24/05/2026
*/

#include "arch/paging.h"

typedef uint32_t pde_t;
typedef uint32_t pte_t;

#define PTE_PRESENT                 (1)
#define PTE_READ_WRITE              (2)
#define PTE_USER                    (4)
#define PTE_WRITE_THROUGH           (8)
#define PTE_CACHE_DISABLED          (16)
#define PTE_ACCESSED                (32)
#define PTE_DIRTY                   (64)
//#define PTE_PAT                     (128)
#define PTE_GLOBAL                  (256)

#define PAGE_DIRECTORY_INDEX(a)     ((a) >> 22)
#define PAGE_TABLE_INDEX(a)         (((a) >> 12) & 0x3ff)
#define PAGE_OFFSET(a)              ((a) & 0xfff)

int join_ptable(cr3_t cr3, int index, void *ptable, unsigned int flags) {
    if (PAGE_OFFSET((uint32_t)ptable))
        return 1;
    
    pde_t *pd_ptr = (pde_t *)(cr3 & ~4095);

    pde_t *pd_entry = &pd_ptr[index];
    
    *pd_entry =
        (pde_t)ptable |
        ((flags & PAGE_PRESENT      ? 1 : 0) << 0) |
        ((flags & PAGE_READ_WRITE   ? 1 : 0) << 1) |
        ((flags & PAGE_USER         ? 1 : 0) << 2) |
        ((flags & PAGE_WRITE_BACK   ? 0 : 1) << 3) |
        ((flags & PAGE_DIRECT       ? 1 : 0) << 4);

    return 0;
}

int map_page(cr3_t cr3, void *vma, void *pma, unsigned int flags) {
    if (PAGE_OFFSET((uint32_t)vma) || PAGE_OFFSET((uint32_t)pma))
        return 1;

    // page directory pointer
    pde_t *pd_ptr = (pde_t *)(cr3 & ~4095);

    // page directory entry
    pde_t pd_entry = pd_ptr[PAGE_DIRECTORY_INDEX((uint32_t)vma)];

    if (!pd_entry)
        return 2;

    if (!(pd_entry & PTE_PRESENT))
        return 3;

    // page table address
    pte_t *pt_ptr = (pte_t *)(pd_entry & ~4095);

    pte_t *pt_entry = &pt_ptr[PAGE_TABLE_INDEX((uint32_t)vma)];

    *pt_entry =
        (pte_t)pma |
        ((flags & PAGE_PRESENT      ? 1 : 0) << 0) |
        ((flags & PAGE_READ_WRITE   ? 1 : 0) << 1) |
        ((flags & PAGE_USER         ? 1 : 0) << 2) |
        ((flags & PAGE_WRITE_BACK   ? 0 : 1) << 3) |
        ((flags & PAGE_DIRECT       ? 1 : 0) << 4);

    return 0;
}