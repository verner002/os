/**
 * @file vmm.c
 * @author verner002
 * @date 23/09/2025
*/

#include "macros.h"
#include "mm/vmm.h"

typedef struct __avl_node AVL_NODE;

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

static bool bitmap_lock = false;
static uint32_t bitmap[256*1024*1024/4096/32];

/**
 * __init_vmm
*/

int32_t __init_vmm(void) {
    __mutex_lock(&bitmap_lock);

    // all pages in pool are free
    for (uint32_t i = 0; i < sizeofarray(bitmap); ++i)
        bitmap[i] = 0;

    __mutex_unlock(&bitmap_lock);
    
    /*root = (AVL_NODE *)e820_malloc(sizeof(AVL_NODE));

    if (!root) {
        printf("Error\n");
        return -1;
    }

    root->start = 0x80000000;
    root->size = 2*1024*1024; // half of the virtual memory
    root->free = true;
    root->height = 0;
    root->left = NULL;
    root->right = NULL;*/
    return 0;
}

/**
 * __map_page
 * 
 * TODO: take page tables from page tables pool
*/

int32_t __map_page(uint32_t virtual_memory, uint32_t physical_memory, uint8_t flags) {
    // FIXME: mapping is done in default address space
    //  use cr3 register to obtain correct page directory
    if (!page_directory || (page_directory & 31) || !physical_memory) {
        printk("page-map: invalid arguments: %p, %p\n", page_directory, physical_memory);
        for(;;);
        return -1;
    }

    virtual_memory &= 0xfffff000;
    physical_memory &= 0xfffff000;
    
    uint32_t page = (uint32_t)virtual_memory / PAGE_SIZE;

    PAGE_DIRECTORY_ENTRY *pde = &((PAGE_DIRECTORY_ENTRY *)page_directory)[page / 1024]; // TODO: use ptr arithmetic?

    PAGE_TABLE_ENTRY *page_table;

    if (!(pde->address & 0x000fffff)) { // no page table, create one
        // TODO:
        //  do not allocate a memory by standard allocation functions
        //  take page from dynamic mapping page pool, if there is no
        //  free page try do swap-out (implement swapping :-)) and if
        //  no page is still available, sleep process
        page_table = (PAGE_TABLE_ENTRY *)pgalloc(PAGE_MAP);

        if (!page_table) {
            printk("page-map: not enough memory for page table\n");
            return -1;
        }

        // clear page table
        memset(page_table, 0, PAGE_TABLE_SIZE);

        pde->address = (uint32_t)page_table >> 12;
        pde->granularity = !!PAGE_4KIB;
        pde->read_write = !!PAGE_READ_WRITE;
        pde->user_supervisor = !!PAGE_USER;
        pde->cache_disabled = 1 /*!!PAGE_CACHE_ENABLED*/;
        pde->write_through = !!PAGE_WRITE_THROUGH;
        pde->present = 1;
    } else
        page_table = (PAGE_TABLE_ENTRY *)(pde->address << 12);

    PAGE_TABLE_ENTRY *pte = &(page_table[page % 1024]);

    // FIXME: do we want this???
    //if (pte->address & 0x000fffff) return -1; // already mapped

    pte->address = ((uint32_t)physical_memory >> 12); // must be page-aligned
    pte->read_write = flags & PAGE_READ_WRITE; // default: read only
    pte->user_supervisor = !!(flags & PAGE_USER); // default: supervisor
    pte->cache_disabled = 1 /*(flags & PAGE_CACHE_DISABLED)*/; // default: cache enabled
    pte->write_through = 1 /*!(flags & PAGE_WRITE_THROUGH)*/; // default: write-through
    pte->page_attribute_table = 0;
    pte->present = 1;

    //asm volatile ("mov cr3, %0\n\t"
    //"mov %1, cr3" : "=r" (page_directory) : "r" (page_directory) : "memory");

    asm volatile (
        "invlpg dword ptr [%0]"
        :
        : "r" (virtual_memory)
        : "memory"
    ); // invalidate page table

    //printk("VAS(%p) -> PAS(%p)\n", virtual_memory, physical_memory);
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
        AVL_NODE *remainder = new_node(node->start + size, node->size - size, true);
        remainder->previous = node;
        node->next = remainder;

        insert_node(root, node->start, size, false);
        insert_node(root, node->start + size, node->size - size, true);
        //delete_node(node);
    }

    if (!node)
        return NULL;

    // split node if larger
    return (void *)node->start;
}

/**
 * map_page
 * 
 * TODO: perform mapping for given length
 * RFC: what to do if there is a request for mapping
 *  that starts in low mem and end in high mem???
*/

void *map_page(void *addr, uint32_t length, uint8_t flags) {
    if (addr + length <= 768*1024*1024)
        return addr; // low memory is directly mapped

    // search, swap pages, search again, on fail sleep
    // process till next swap and repeate

    while (true) {
        __mutex_lock(&bitmap_lock);
        
        for (uint32_t i = 0; i < sizeofarray(bitmap); ++i) {
            if (~bitmap[i]) {
                for (uint32_t j = 0; j < 32; ++j)
                    if (!(bitmap[i] & (1 << j))) {
                        bitmap[i] |= (1 << j);
                        __mutex_unlock(&bitmap_lock);
                        // TODO: map the page
                        return (void *)((i * 32 + j) * 4096 + 768*1024*1024/* + 0xc0000000*/);
                    }
            }
        }

        __mutex_unlock(&bitmap_lock);

        // TODO: swap and repeate
        // TODO: sleep process
    }
}

/**
 * unmap_page
*/

int32_t unmap_page(void *addr, uint32_t length) {
    __mutex_lock(&bitmap_lock);
    uint32_t address = ((uint32_t)addr - 768*1024*1024) / 4096;
    // TODO: unmap the page
    bitmap[address / 32] &= ~(1 << (address % 32));
    __mutex_unlock(&bitmap_lock);
    return 0;
}

/**
 * __memcore
*/

void __memcore(void) {
    // enlarge the heap (realized using linked-list)
}