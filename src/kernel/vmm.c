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
 * Types Definitions
*/

typedef struct __avl_node AVL_NODE;

/**
 * Structures
*/

// let's use avl instead of red-black trees
// we'll be doing many and many lookups and
// that's what are avl trees good at
struct __avl_node {
    uint32_t start; // base address
    uint32_t size; // size in pages
    bool free;
    AVL_NODE *left;
    AVL_NODE *right;
};

/**
 * Global Static Variables
*/

// TODO: only for one page directory!!!
static AVL_NODE *root;

/**
 * Global Variables
*/

//CR3 page_directory;
uint32_t page_directory;

/**
 * __init_vmm
*/

int32_t __init_vmm(void) {
    printk("\033[33mvmm:\033[37m Initializing... ");

    root = (AVL_NODE *)malloc(sizeof(AVL_NODE));

    if (!root) {
        printf("Error\n");
        return -1;
    }

    root->start = 0;
    root->size = 1024*1024;
    root->free = TRUE;
    root->left = NULL;
    root->right = NULL;

    printf("Ok\n");
    return 0;
}

/**
 * __map_page
*/

int32_t __map_page(void *virtual_memory, void *physical_memory, uint8_t flags) {
    if (!page_directory || (page_directory & 31) || !physical_memory) return -1;
    
    uint32_t page = (uint32_t)virtual_memory / 4096;

    PAGING_DIRECTORY_ENTRY *pde = &((PAGING_DIRECTORY_ENTRY *)page_directory)[page / 1024]; // TODO: use ptr arithmetic?

    PAGING_TABLE_ENTRY *page_table;

    if (!(pde->address & 0x000fffff)) { // no page table, create one
        // we could use pgalloc but we don't need aligned page,
        // just 4096 bytes
        page_table = (PAGING_TABLE_ENTRY *)e820_rmalloc(4096, TRUE);

        if (!page_table) return -1; // allocation failed

        memset(page_table, 0, 4096);

        pde->address = (uint32_t)page_table >> 12;
        pde->granularity = PAGE_4KIB;
        pde->read_write = (bool)PAGE_READ_WRITE;
        pde->user_supervisor = (bool)PAGE_USER;
        pde->cache_disabled = PAGE_CACHE_ENABLED;
        pde->write_through = (bool)PAGE_WRITE_THROUGH;
        pde->present = TRUE;
    } else page_table = (PAGING_TABLE_ENTRY *)(pde->address << 12);

    PAGING_TABLE_ENTRY *pte = &(page_table[page % 1024]);

    // FIXME: do we wat this???
    //if (pte->address & 0x000fffff) return -1; // already mapped

    pte->address = ((uint32_t)physical_memory >> 12); // must be page-aligned
    pte->read_write = flags & PAGE_READ_WRITE; // default: read only
    pte->user_supervisor = flags & PAGE_USER; // default: supervisor
    pte->cache_disabled = flags & PAGE_CACHE_DISABLED; // default: cache enabled
    pte->write_through = !(flags & PAGE_WRITE_THROUGH); // default: write-through
    pte->page_attribute_table = 0;
    pte->present = TRUE;

    //asm volatile ("mov cr3, %0\n\t"
    //"mov %1, cr3" : "=r" (page_directory) : "r" (page_directory) : "memory");

    asm volatile (
        //"cli\t\n"
        "invlpg dword ptr [%0]\t\n"
        //"sti"
        :
        : "r" (virtual_memory)
        : "memory"
    ); // invalidate page table

    return 0;
}

/**
 * __find_best_fit
*/

static AVL_NODE *__find_best_fit(AVL_NODE *node, uint32_t size) {
    // let's use best-fit strategy
    if (node->size < size) return __find_best_fit(node->right, size);
    else if (node->size > size) {
        AVL_NODE *better_node = __find_best_fit(node->left, size);
        return better_node ? better_node : node;
    }
    
    return node;
}

/**
 * __alloc_region
*/

int32_t __alloc_region(uint32_t size) {
    
}

/**
 * __mmap
*/

void *__mmap(void *physical_memory, uint32_t length, uint8_t flags) {

}