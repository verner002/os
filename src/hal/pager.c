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

void __map_page(dword virt_addr, dword phys_addr, byte avl, bool g, bool d, bool pcd, bool pwt, bool us, bool rw, bool p) {
    asm (
        "invlpg [ebx]\n\t" // invalidate page table entry
        :
        : "b" (phys_addr)
        :
    );
}

/**
 * __alloc_pages
*/

byte *__alloc_page(unsigned int c) {
    return NULL;
}