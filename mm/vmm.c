/**
 * @file vmm.c
 * @author verner002
 * @date 18/03/2026
*/

#include "macros.h"
#include "mm/pfa.h"
#include "kstdlib/math.h"
#include "mm/vmm.h"
#include "kernel/panic.h"

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
 * map_page
 * 
 * TODO: perform mapping for given length
 * RFC: what to do if there is a request for mapping
 *  that starts in low mem and end in high mem???
*/

void *map_page(void *addr, uint32_t length, uint8_t flags) {
    if (((uint32_t)addr + length) <= 768*1024*1024)
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

/*int tlb_flush_all(struct vm_area *area) {
    asm volatile (
        "mov cr3, %0"
        :
        : "r" (task->cr3)
        : "memory"
    );
}*/

// start is the beginning of the area to be flushed
// end is calculated this way: start + length - 1
int tlb_flush_range(struct vm_area *area, void *start, void *end) {
    // for example range 0x00000000 - 0x000017ff
    // will be aligned to 0x00000000 - 0x00002000
    uint32_t vpage_start = (uint32_t)start & ~4095;
    uint32_t vpage_end = ALIGN_UP((uint32_t)end + 1, PAGE_SIZE);

    if (vpage_start < area->vm_start || vpage_end > area->vm_end)
        return -1; // out of vm area

    while (vpage_start < vpage_end) {
        asm volatile (
            "invlpg [%0]"
            :
            : "r" (vpage_start)
            : "memory"
        ); // invalidate page

        vpage_start += 4096;
    }

    return 0;
}

/**
 * kmap
 * 
 * Maps virtual memory to physical memory.
 * Always starts from the nearest lower page
 * boundary to the nearest higher page boundary.
*/

int kmap(pg_dir pdir, void *virt, void *phys, int length, int flags) {
    if (!pdir)
        return -1; // invalid argument

    uint32_t vpage_addr = (uint32_t)virt & ~4095;
    uint32_t vpage = vpage_addr / 4096;
    uint32_t ppage_addr = (uint32_t)phys & ~4095;
    uint32_t ppage = ppage_addr / 4096;
    uint32_t pages = (length + 4095) / 4096;

    //spin_lock(&pdir->lock);

    for (int i = 0; i < pages; ++i) {
        pg_dir_entry *pdir_entry = &pdir[vpage / 1024];
        
        pg_tbl ptbl = (pg_tbl)(pdir_entry->address << 12);

        if (!ptbl) {
            // request page table from pool
            ptbl = pgalloc(0);
            memset(ptbl, 0, PAGE_SIZE);
            printk("page table not exist! creating...\n");
            //return -2;
        } else if (!ptbl->present) { // FIXME: ptbl does not contain all fields
            // page table exists but is not present
            // in the mmeory
            printk("page table not in memory\n");
            return -3;
        }

        pg_tbl_entry *ptbl_entry = &ptbl[vpage % 1024];

        // use transaction so that when we encounter
        // "page in use" no chages are made to page tables and page directory
        if (ptbl_entry) {
            //spin_unlock(&pdir->lock);
            printk("page in use (%p)\n", ptbl_entry);
            return -4; // page in use
        }

        ptbl_entry->present = 1;
        ptbl_entry->read_write = (flags & PG_READ_WRITE) ? 1 : 0;
        ptbl_entry->user_supervisor = (flags & PG_USER) ? 1 : 0;
        ptbl_entry->write_through = (flags & PG_WRITE_BACK) ? 0 : 1;
        ptbl_entry->cache_disabled = (flags & PG_CACHE_DISABLED) ? 1 : 0;
        ptbl_entry->page_attribute_table = 0;
        ptbl_entry->address = ppage_addr >> 12;

        ++vpage;
        ppage_addr += 4096;
    }

    //spin_unlock(&pdir->lock);
    return 0;
}

int vm_avl_height(struct vm_area *node) {
    if (!node)
        return 0;

    return node->avl_height;
}

int vm_avl_balance_factor(struct vm_area *node) {
    return vm_avl_height(node->vm_left) - vm_avl_height(node->vm_right);
}

void vm_avl_update_height(struct vm_area *node) {
    if (!node)
        return;

    node->avl_height = max(vm_avl_height(node->vm_left), vm_avl_height(node->vm_right)) + 1;
}

struct vm_area *rotate_right(struct vm_area *node) {
    struct vm_area *left = node->vm_left;
    node->vm_left = left->vm_right;
    left->vm_right = node;
    vm_avl_update_height(node);
    vm_avl_update_height(left);
    return left;
}

struct vm_area *rotate_left(struct vm_area *node) {
    struct vm_area *right = node->vm_right;
    node->vm_right = right->vm_left;
    right->vm_left = node;
    vm_avl_update_height(node);
    vm_avl_update_height(right);
}

struct vm_area *vm_avl_rebalance(struct vm_area *node) {
    vm_avl_update_height(node);

    int balance = vm_avl_balance_factor(node);

    if (balance < -1) {
        if (vm_avl_balance_factor(node->vm_right) > 0)
            node->vm_right = rotate_right(node->vm_right);

        return rotate_left(node);
    } else if (balance > 1) {
        if (vm_avl_balance_factor(node->vm_left) < 0)
            node->vm_left = rotate_left(node->vm_left);

        return rotate_right(node);
    }

    return node;
}

// look-up by address - we find the range the
// address belongs to
struct vm_area *vm_avl_lookup_addr(struct vm_area *root, unsigned long addr) {
    if (!root)
        return NULL;

    if (addr > root->vm_end)
        return vm_avl_lookup_addr(root->vm_right, addr);
    else if (addr < root->vm_start)
        return vm_avl_lookup_addr(root->vm_left, addr);

    return root;
}

// splits the node into two based on addr value, first area
// will end at addr, second will start at addr+1
struct vm_area *vm_avl_split(struct vm_area *root, struct vm_area *node, unsigned long addr, struct vm_area **remainder) {
    if (addr < node->vm_start || node->vm_end < addr)
        return NULL; // split failed

    if (addr == node->vm_start || addr == node->vm_end)
        return root; // nothing to do

    // allocation needs to be done in kernel address space
    // (same for all tasks) since otherwise this could lead
    // to chicken-egg problem
    struct vm_area *piece = (struct vm_area *)kmalloc(sizeof(struct vm_area));

    if (!piece)
        return NULL; // split failed

    piece->vm_start = addr + 1;
    piece->vm_end = node->vm_end;
    piece->vm_left = NULL;
    piece->vm_right = node->vm_right;

    if (remainder)
        *remainder = piece;

    node->vm_end = addr;
    node->vm_right = piece;

    return vm_avl_rebalance(root);
}

struct vm_area *vm_avl_insert(struct vm_area *root, struct vm_area *node) {
    if (!root)
        return node; // tree is empty, node becomes root

    unsigned long node_key = node->vm_start;

    if (node_key > root->vm_end) { // node goes to right subtree
        root->vm_right = vm_avl_insert(root->vm_right, node);

        if (root->vm_right == node) {
            struct vm_area *temp = root->vm_next;
            root->vm_next = node;
            
            if (temp)
                temp->vm_prev = node;

            node->vm_prev = root;
            node->vm_next = temp;
        }
    } else if (node_key < root->vm_start) { // node goes to left subtree
        root->vm_left = vm_avl_insert(root->vm_left, node);

        if (root->vm_left == node) {
            struct vm_area *temp = root->vm_prev;
            root->vm_prev = node;
            
            if (temp)
                temp->vm_next = node;

            node->vm_next = root;
            node->vm_prev = temp;
        }
    } else // node is already present
        return root;

    // restore invariance
    return vm_avl_rebalance(root);
}

struct vm_area *reserve_area(struct vm_area **root, unsigned long start, unsigned long end) {
    struct vm_area *area = vm_avl_lookup_addr(*root, start);

    // check if address exists and is in one area
    if (!area || area != vm_avl_lookup_addr(*root, end))
        return NULL;

    if (start > area->vm_start) {
        // TODO: check root
        *root = vm_avl_split(*root, area, start - 1, &area);
    }

    if (end < area->vm_end) {
        // TODO: check root, repeate/panic
        *root = vm_avl_split(*root, area, end, NULL);
    }

    return area;
}

struct vm_area *first_fit_alloc(struct vm_area **root, unsigned long size) {
    struct vm_area *left_most = *root;

    while (left_most->vm_left)
        left_most = left_most->vm_left;

    do {
        unsigned long prev_end = left_most->vm_prev ? left_most->vm_prev->vm_end : (unsigned long)-1;
    
        if (left_most->vm_start - prev_end - 1 >= size) {
            struct vm_area *new = (struct vm_area *)kmalloc(sizeof(struct vm_area));

            if (!new) {
                printk("kernel: vmm: %s: out of memory\n", __func__);
                return VMA_INVALID;
            }

            new->avl_height = 1;
            new->task = thread_current;
            new->vm_start = prev_end + 1;
            new->vm_end = new->vm_start + size - 1;
            new->vm_left = NULL;
            new->vm_right = NULL;
            new->vm_prev = NULL;
            new->vm_next = NULL;
            *root = vm_avl_insert(*root, new);
            return new;
        }

        left_most = left_most->vm_next;
    } while (left_most);

    return VMA_INVALID;
}

/**
 * alloc_area
 * 
 * Suitable for large (page-aligned) allocations.
 * Allocated virtual memory is not mapped to the
 * physical memory until access or specified by
 * map and flags.
*/

struct vm_area *alloc_area(struct vm_area **root, pg_dir pdir, unsigned long size, bool map, int flags) {
    size = ALIGN_UP(size, PAGE_SIZE);

    struct vm_area *area = first_fit_alloc(root, size);

    if (area == VMA_INVALID)
        return VMA_INVALID;

    if (map) {
        void *vm_addr = (void *)area->vm_start;
        int pages = size / PAGE_SIZE;

        for (int i = 0; i < pages; ++i) {
            void *ph_addr = pgalloc(0);
            int result = kmap(pdir, vm_addr, ph_addr, PAGE_SIZE, flags);

            // TODO: rollback(?)
            if (result)
                printk("failed to map virt(%p) -> phys(%p), code %i\n", vm_addr, ph_addr, result);

            vm_addr += PAGE_SIZE;
        }
    }

    return area;
}

void *alloc_pg_dir(void) {
    void *pdir = pgalloc(0);
    
    if (!pdir)
        return NULL;

    memset(pdir, 0, PAGE_SIZE);
    return pdir;
}

__attribute__((interrupt)) void page_fault_handler(__attribute__((unused)) struct __interrupt_frame *frame) {
    uint32_t vas;

    asm volatile (
        "mov eax, cr2"
        : "=a" (vas)
        :
        :
    );

    if (thread_current && __get_pid() != 0) {
        sti();

        struct vm_area **root = get_vm_root_ptr();

        struct vm_area *area = vm_avl_lookup_addr(*root, vas);

        if (area) {
            printf("Segmentation fault\n");
            __exit(-1);
        }

        printk("virtual memory area exists but not mapped\n");
        __exit(-1);
    }

    printk("page-fault when accessing address %p\n", vas);
    panic();
}