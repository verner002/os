/**
 * File Allocation Table 12
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "types.h"
#include "drivers/block/82077aa.h"
#include "mm/e820.h"
#include "kstdlib/string.h"
#include "hal/vfs.h"

#define FAT12_ATTRIBUTE_READONLY 0x01
#define FAT12_ATTRIBUTE_HIDDEN 0x02
#define FAT12_ATTRIBUTE_SYSTEM 0x04
#define FAT12_ATTRIBUTE_VOLUME 0x08
#define FAT12_ATTRIBUTE_SUBDIR 0x10
#define FAT12_ATTRIBUTE_ARCHIVE 0x20

#define FAT12_NAME_LENGTH 8
#define FAT12_EXT_LENGTH 3
#define FAT12_DIRNAME_LENGTH FAT12_NAME_LENGTH
#define FAT12_FILENAME_LENGTH (FAT12_NAME_LENGTH + FAT12_EXT_LENGTH)

#define VFAT_ORDINAL 0x3f
#define VFAT_LAST_LFN_ENTRY 0x40
#define VFAT_DELETED_LFN_ENTRY 0x80

#define VFAT_LFN_CHARS1 5
#define VFAT_LFN_CHARS2 6
#define VFAT_LFN_CHARS3 2
#define VFAT_LFN_CHARS (VFAT_LFN_CHARS1 + VFAT_LFN_CHARS2 + VFAT_LFN_CHARS3)

/**
 * Types Definitions
*/

typedef struct __fat12_record FAT12_RECORD;
typedef struct __vfat_lfn_entry VFAT_LFN_ENTRY;

/**
 * Structures
*/

struct __attribute__((__packed__)) __fat12_record {
    // 8.3 format
    char filename[8];
    char extension[3];
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

struct __attribute__((__packed__)) __vfat_lfn_entry {
    uint8_t ordinal;
    uint16_t chars1[5];
    uint8_t flags;
    uint8_t reserved1;
    uint8_t checksum;
    uint16_t chars2[6];
    uint16_t reserved2;
    uint16_t chars3[2];
};

/**
 * Declarations
*/

int32_t __fat12_read_fat(void);
int32_t __fat12_read_root_dir(void);
bool __fat12_file_exists(char const *filename);
int32_t __fat12_load_file(char const *filename, uint32_t buffer);
int32_t __fat12_list_rootdir(void);
//int32_t __fat12_list_dir(VFS_DIR_NODE *node);