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

// we could use the paging directory and
// the paging tables but that would be too
// slow to use, avl trees are much better
// let's use avl instead of red-black trees
// we'll be doing many and many lookups and
// that's what avl trees are good at
struct __avl_node {
    uint32_t start; // base address
    uint32_t size; // size in pages
    bool free;
    int32_t height;
    AVL_NODE *left;
    AVL_NODE *right;
    AVL_NODE *previous;
    AVL_NODE *next;
    uint32_t free_c;
    AVL_NODE *free_n; // free nodes with the same size
    uint32_t used_c;
    AVL_NODE *used_n;
};

/**
 * Global Static Variables
*/

// TODO: only for one page directory, keep track of all avl trees!!!
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

    /*root = (AVL_NODE *)e820_malloc(sizeof(AVL_NODE));

    if (!root) {
        printf("Error\n");
        return -1;
    }

    root->start = 0x80000000;
    root->size = 2*1024*1024; // half of the virtual memory
    root->free = TRUE;
    root->height = 0;
    root->left = NULL;
    root->right = NULL;*/

    printf("Ok\n");
    return 0;
}

/**
 * __map_page
*/

int32_t __map_page(uint32_t virtual_memory, uint32_t physical_memory, uint8_t flags) {
    if (!page_directory || (page_directory & 31) || !physical_memory) return -1;
    
    uint32_t page = (uint32_t)virtual_memory / PAGE_SIZE;

    PAGING_DIRECTORY_ENTRY *pde = &((PAGING_DIRECTORY_ENTRY *)page_directory)[page / 1024]; // TODO: use ptr arithmetic?

    PAGING_TABLE_ENTRY *page_table;

    if (!(pde->address & 0x000fffff)) { // no page table, create one
        page_table = (PAGING_TABLE_ENTRY *)e820_rmalloc(4096, TRUE)/*pgalloc()*/;

        memset(page_table, 0, PAGE_TABLE_SIZE);

        pde->address = (uint32_t)page_table >> 12;
        pde->granularity = PAGE_4KIB;
        pde->read_write = (bool)PAGE_READ_WRITE;
        pde->user_supervisor = (bool)PAGE_USER;
        pde->cache_disabled = PAGE_CACHE_ENABLED;
        pde->write_through = (bool)PAGE_WRITE_THROUGH;
        pde->present = TRUE;
    } else page_table = (PAGING_TABLE_ENTRY *)(pde->address << 12);

    PAGING_TABLE_ENTRY *pte = &(page_table[page % 1024]);

    // FIXME: do we want this???
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
 * __height
*/

int32_t __height(AVL_NODE *node) {
    if (!node)
        return 0;

    return node->height;
}

/**
 * __balance
*/

int32_t __balance(AVL_NODE *node) {
    /*if (!node)
        return 0;*/

    return __height(node->left) - __height(node->right);
}

/**
 * __left_left_rotation
*/

static AVL_NODE *__left_left_rotation(AVL_NODE *parent) {
    AVL_NODE *child = parent->right;
    parent->right = child->left;
    child->left = parent;
    
    parent->height = max(
        __height(parent->left),
        __height(parent->right)
    ) + 1;

    child->height = max(
        __height(child->left),
        __height(child->right)
    ) + 1;

    return child; // return new root
}

/**
 * __right_right_rotation
*/

static AVL_NODE *__right_right_rotation(AVL_NODE *parent) {
    AVL_NODE *child = parent->left;
    parent->left = child->right;
    child->right = parent;

    parent->height = max(
        __height(parent->left),
        __height(parent->right)
    ) + 1;

    child->height = max(
        __height(child->left),
        __height(child->right)
    ) + 1;

    return child; // return new root
}

/**
 * __left_right_rotation
*/

static AVL_NODE *__left_right_rotation(AVL_NODE *parent) {
    parent->left = __left_left_rotation(parent->left);
    return __right_right_rotation(parent);
}

/**
 * __right_left_rotation
*/

static AVL_NODE *__right_left_rotation(AVL_NODE *parent) {
    parent->right = __right_right_rotation(parent->right);
    return __left_left_rotation(parent);
}

static AVL_NODE *new_node(uint32_t start, uint32_t size, bool free) {
    AVL_NODE *node = (AVL_NODE *)kmalloc(sizeof(AVL_NODE));

    if (!node)
        return NULL;

    node->start = start;
    node->size = size;
    node->free = free;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    node->previous = NULL;
    node->next = NULL;
    node->free_c = 0;
    node->free_n = NULL;
    node->used_c = 0;
    node->used_n = NULL;

    return node;
}

AVL_NODE *insert_node(AVL_NODE *node, uint32_t start, uint32_t size, bool free) {
    if (!node) 
        return new_node(start, size, free);

    uint32_t node_size = node->size;

    if (size > node_size)
        node->right = insert_node(node->right, start, size, free);
    else if (size < node_size)
        node->left = insert_node(node->left, start, size, free);
    else {
        AVL_NODE *item = new_node(start, size, free);

        if (free) {
            item->right = node->free_n;
            node->free_n = item;
            ++node->free_c;
        } else {
            item->right = node->used_n;
            node->used_n = item;
            ++node->used_c;
        }

        return node;
    }

    node->height = max(
        __height(node->left),
        __height(node->right)
    ) + 1;

    int balance = __balance(node);

    if (balance > 1) {
        if (size < node->left->size)
            return __right_right_rotation(node);
        else
            return __left_right_rotation(node);
    } else if (balance < -1) {
        if (size > node->right->size)
            return __left_left_rotation(node);
        else
            return __right_left_rotation(node);
    }   

    // balance is within the interval [-1; 1]
    return node;
}

AVL_NODE *delete_node(AVL_NODE *node, AVL_NODE *nodeToBeDeleted) {
    if (!node)
        return NULL; // not found

    if (nodeToBeDeleted->size > node->size)
        delete_node(node->right, nodeToBeDeleted);
    else if (nodeToBeDeleted->size < node->size)
        delete_node(node->left, nodeToBeDeleted);
    else { // we've found the node with the same size
        if (node->left && node->right) {
            // find inorder successor in right subtree
        } else if (node->left) {
            // only left child
        } else if (node->right) {
            // only right child
        } else {
            // no child
        }
    }
}

/**
 * __find_best_fit_region
*/

static AVL_NODE * __find_best_fit_region(AVL_NODE *node, uint32_t size) {
    AVL_NODE *child;

    // let's use best-fit strategy
    if (!node) return NULL;
    else if (node->size < size) child = __find_best_fit_region(node->right, size);
    else if (node->size > size) child = __find_best_fit_region(node->left, size);
    else child = node;
    
    return child ? child : node;
}

/**
 * __find_best_fit
*/

static AVL_NODE *__find_best_fit(uint32_t size) {
    return __find_best_fit_region(root, size);
}

/**
 * __alloc
*/

void *__alloc(uint32_t size) {
    AVL_NODE *node = __find_best_fit(size);

    if (node->free_c > 0) {
        AVL_NODE *free_n = node->free_n->right;
        node->free_n->right = node->used_n;
        node->used_n = node->free_n;
        node->free_n = free_n;
        --node->free_c;
        ++node->used_c;
        node = node->used_n;
    }

    if (node > size) { // node > size + 16
        // split
        AVL_NODE *remainder = new_node(node->start + size, node->size - size, TRUE);
        remainder->previous = node;
        node->next = remainder;

        insert_node(root, node->start, size, FALSE);
        insert_node(root, node->start + size, node->size - size, TRUE);
        //delete_node(node);
    }

    if (!node)
        return NULL;

    // split node if larger
    return (void *)node->start;
}

/**
 * __mmap
*/

void *__mmap(void *physical_memory, uint32_t length, uint8_t flags) {
    // get the first free va with required length
}

/**
 * __memcore
*/

void __memcore(void) {
    // enlarge the heap (realized using linked-list)
}