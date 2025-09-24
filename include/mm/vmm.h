/**
 * Virtual Memory Manager
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "types.h"
#include "mm/e820.h"
#include "mm/pager.h"
#include "kstdlib/stdio.h"
#include "mm/heap.h"
#include "kstdlib/math.h"

/**
 * Types Definitions
*/

typedef struct __cr3 CR3;
typedef struct __paging_directory_entry PAGING_DIRECTORY_ENTRY;
typedef struct __paging_table_entry PAGING_TABLE_ENTRY;

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

struct __attribute__((__packed__)) __paging_directory_entry {
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

struct __attribute__((__packed__)) __paging_table_entry {
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

//extern CR3 page_directory;
extern uint32_t page_directory;

/**
 * Declarations
*/

int32_t __init_vmm(void);
int32_t __map_page(uint32_t virtual_memory, uint32_t physical_memory, uint8_t flags);
//void *__alloc(uint32_t size);
void *__mmap(void * physical_memory, uint32_t length, uint8_t flags);