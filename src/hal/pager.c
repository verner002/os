/**
 * Pager
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/pager.h"

/**
 * Static Global Variables
*/

/**
 * __map_page
*/

void __map_page(uint32_t virt_addr, uint32_t phys_addr, uint8_t avl, bool g, bool d, bool pcd, bool pwt, bool us, bool rw, bool p) {
    asm (
        "invlpg [ebx]" // invalidate page table entry
        :
        : "b" (phys_addr)
        :
    );
}

/**
 * __alloc_pages
*/

uint8_t *__alloc_page(uint32_t c) {
    return NULL;
}