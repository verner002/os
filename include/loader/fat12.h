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

struct _record {
    byte filename[11]; // 8.3 format
    byte attributes;
    byte _reserved1[2];
    word creation_time;
    word creation_date;
    word last_access_date;
    byte _reserved2[2];
    word last_write_time;
    word last_write_date;
    word first_cluster;
    dword file_size;
};