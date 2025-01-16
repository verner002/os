/**
 * File Allocation Table 12
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"
//#include "kstdlib/stdlib.h"

/**
 * Types Definitions
*/

typedef enum _attributes ATTRIBUTES;
typedef struct _fat FAT;
typedef struct _record RECORD;

/**
 * Enumerations
*/

enum _attributes {
    READ_ONLY = 0x01,
    HIDDEN = 0x02,
    SYSTEM = 0x04,
    VOLUME_LABEL = 0x08,
    SUBDIRECTORY = 0x10,
    ARCHIVE = 0x20/*,
    _UNUSED1 = 0x40,
    _UNUSED2 = 0x80*/
};

/**
 * Structures
*/

struct _fat {
    uint16_t sectorsPerTrack;
    uint16_t headsPerCylinder;
};

struct _record {
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

void __lba2chs(uint32_t lba, uint16_t *c, uint16_t *h, uint16_t *s);