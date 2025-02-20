/**
 * File Allocation Table 12
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "types.h"
#include "drivers/82077aa.h"
#include "kernel/e820.h"
#include "kstdlib/string.h"

/**
 * Types Definitions
*/

typedef struct __fat12_record FAT12_RECORD;

/**
 * Structures
*/

struct __fat12_record {
    uint8_t filename[11]; // 8.3 format
    uint8_t attributes;
    uint8_t _reserved1[2];
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint8_t _reserved2[2];
    uint16_t last_write_time;
    uint16_t last_write_date;
    uint16_t first_cluster;
    uint32_t file_size;
};

/**
 * Declarations
*/

uint32_t __fat12_read_fat(void);
uint32_t __fat12_read_root_dir(void);
uint32_t __fat12_load_file(char const *filename, uint32_t buffer);