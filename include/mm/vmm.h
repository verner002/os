/**
 * @file vmm.h
 * @author verner002
 * @date 18/03/2026
*/

#pragma once

#include <assert.h>
#include "types.h"
#include "mm/e820.h"
#include "mm/pfa.h"
#include "kstdlib/stdio.h"
#include "mm/heap.h"
#include "kstdlib/math.h"

#define PG_READ_ONLY        0
#define PG_READ_WRITE       1
#define PG_SUPERVISOR       0
#define PG_USER             2
#define PG_WRITE_THROUGH    0
#define PG_WRITE_BACK       4
#define PG_CACHE_ENABLED    0
#define PG_CACHE_DISABLED   8
#define PG_NOT_PRESENT      0
#define PG_PRESENT          16

#define VMA_INVALID         ((struct vm_area *)-1)

typedef struct __attribute__((__packed__)) {
    uint32_t present                : 1;
    uint32_t read_write             : 1;
    uint32_t user_supervisor        : 1;
    uint32_t write_through          : 1;
    uint32_t cache_disabled         : 1;
    uint32_t accessed               : 1;
    uint32_t dirty                  : 1;
    uint32_t page_attribute_table   : 1;
    uint32_t global                 : 1;
    uint32_t available              : 3; // os specific use
    uint32_t address                : 20; // physical address
} pg_tbl_entry;

typedef pg_tbl_entry * pg_tbl;

/*typedef struct {
    pg_tbl_entry entries[1024];
    //spinlock_t lock;
} pg_tbl;*/

typedef struct __attribute__((__packed__)) {
    uint32_t present                : 1;
    uint32_t read_write             : 1;
    uint32_t user_supervisor        : 1;
    uint32_t write_through          : 1;
    uint32_t cache_disabled         : 1;
    uint32_t accessed               : 1;
    uint32_t available1             : 1; // os specific use
    uint32_t granularity            : 1;
    uint32_t available2             : 4; // os specific use
    uint32_t address                : 20; // page table address
} pg_dir_entry;

typedef pg_dir_entry * pg_dir;

struct __attribute__((packed)) cr3 {
    uint32_t reserved1          : 3;
    uint32_t pwt                : 1; // page-level write-through
    uint32_t pcd                : 1; // page-level cache disable
    uint32_t reserved2          : 7;
    uint32_t pdbr               : 20; // page directory base register
};

static_assert(sizeof(struct cr3) == sizeof(uint32_t), "");

struct vm_area {
    struct task *task;

    unsigned long vm_start;
    unsigned long vm_end;

    int flags;

    // tree height (including node itself)
    int avl_height;

    // avl tree nodes
    struct vm_area *vm_left;
    struct vm_area *vm_right;

    // these are invariants
    // when rotating the tree

    // most right left node
    struct vm_area *vm_prev;
    // most left right node
    struct vm_area *vm_next;

    spinlock_t lock;
};

/*typedef struct {
    pg_dir_entry entries[1024];
    //spinlock_t lock;
} pg_dir;*/

int kmap(pg_dir pdir, void *virt, void *phys, int length, int flags);
struct vm_area *alloc_area(struct vm_area **root, pg_dir pdir, unsigned long size, bool map, int flags);

/**
 * Types Definitions
*/

typedef struct __cr3 CR3;
typedef struct __page_directory_entry PAGE_DIRECTORY_ENTRY;
typedef struct __page_table_entry PAGE_TABLE_ENTRY;

/**
 * Constants
*/

#define PAGE_SIZE 4096
#define PAGE_TABLE_SIZE 4096

/**
 * Enumerations
*/

enum __page_flags {
    PAGE_NONE = 0,
    PAGE_CACHE_ENABLED = 0,
    PAGE_WRITE_BACK = 0,
    PAGE_4KIB = 0,
    PAGE_READ_WRITE = 1,
    PAGE_USER = 2,
    PAGE_4MIB = 4,
    PAGE_CACHE_DISABLED = 8,
    PAGE_WRITE_THROUGH = 16
};

/**
 * Structures
*/

/*struct __attribute__((__packed__)) __cr3 {
    uint32_t reserved1          : 3;
    uint32_t write_through      : 1;
    uint32_t cache_disabled     : 1;
    uint32_t address            : 27;
};*/

struct __attribute__((__packed__)) __page_directory_entry {
    uint32_t present            : 1;
    uint32_t read_write         : 1;
    uint32_t user_supervisor    : 1;
    uint32_t write_through      : 1;
    uint32_t cache_disabled     : 1;
    uint32_t accessed           : 1;
    uint32_t available1         : 1; // os specific use
    uint32_t granularity        : 1;
    uint32_t available2         : 4; // os specific use
    uint32_t address            : 20; // paging table address
};

struct __attribute__((__packed__)) __page_table_entry {
    uint32_t present                : 1;
    uint32_t read_write             : 1;
    uint32_t user_supervisor        : 1;
    uint32_t write_through          : 1;
    uint32_t cache_disabled         : 1;
    uint32_t accessed               : 1;
    uint32_t dirty                  : 1;
    uint32_t page_attribute_table   : 1;
    uint32_t global                 : 1;
    uint32_t available              : 3; // os specific use
    uint32_t address                : 20; // physical address
};

/**
 * Global Variables
*/

extern uint32_t page_directory;

/**
 * Declarations
*/

int32_t __init_vmm(void);
int32_t __map_page(uint32_t virtual_memory, uint32_t physical_memory, uint8_t flags);
//void *__alloc(uint32_t size);
void *map_page(void *addr, uint32_t length, uint8_t flags);
int32_t unmap_page(void *addr, uint32_t length);