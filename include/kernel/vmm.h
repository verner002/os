/**
 * Virtual Memory Manager
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "types.h"
#include "kstdlib/stdio.h"

/**
 * Types Definitions
*/

typedef struct __paging_directory_entry PAGING_DIRECTORY_ENTRY;
typedef struct __paging_table_entry PAGING_TABLE_ENTRY;

/**
 * Enumerations
*/

enum __page_flags {
    PAGE_READ_WRITE = 0x01,
    PAGE_USER = 0x02
};

/**
 * Structures
*/

struct __attribute__((__packed__)) __paging_directory_entry {
    uint32_t present            : 1;
    uint32_t read_write         : 1;
    uint32_t user_supervisor    : 1;
    uint32_t write_through      : 1;
    uint32_t cache_disabled     : 1;
    uint32_t accessed           : 1;
    uint32_t available1         : 1; // os specific use
    uint32_t granularity        : 1;
    uint32_t available2         : 4; // is specific use
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
 * Declarations
*/

uint32_t __mmap(PAGING_DIRECTORY_ENTRY *paging_directory, void *virtual_memory, void *physical_memory, uint8_t flags);