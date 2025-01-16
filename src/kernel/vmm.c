/**
 * Virtual Memory Manager
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/vmm.h"

/**
 * __mmap
*/

uint32_t __mmap(PAGING_DIRECTORY_ENTRY *paging_directory, void *virtual_memory, void *physical_memory, uint8_t flags) {
    uint32_t page = (uint32_t)virtual_memory / 4096;

    PAGING_DIRECTORY_ENTRY *pde = &paging_directory[page / 1024]; // TODO: use ptr arithmetic?

    if (!pde->present) {
        /**
         * create new page table
         * use e820_alloc (given memory can be outside mapped area)
         *  => implement function that will return free area within first MiB
        */

       printk("\033[31m" __FILE__ ":%s:%u: not yet implemented\033[37m\n", __func__, __LINE__);
       return -1;
    }

    PAGING_TABLE_ENTRY *pte = &((PAGING_TABLE_ENTRY *)pde->address)[page % 1024];

    if (pte->present) return -1; // already mapped

    pte->address = (uint32_t)physical_memory & 0xfffff000; // must be page-aligned
    pte->read_write = flags & PAGE_READ_WRITE;
    pte->user_supervisor = flags & PAGE_USER;
    pte->present = 1;
}