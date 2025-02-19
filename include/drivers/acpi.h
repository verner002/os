/**
 * Advanced Configuration & Power Interface
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "null.h"
#include "types.h"
#include "kernel/e820.h"
#include "kernel/vmm.h"
#include "kstdlib/stdio.h"

/**
 * Types Definitions
*/

typedef struct __rsdp RSDP;
typedef struct __xsdp XSDP;
typedef struct __sdt_header SDT_HEADER;
typedef struct __sdt SDT;

/**
 * Structures
*/

struct __attribute__((__packed__)) __rsdp {
    uint8_t signature[8]; // 'RSD PTR '
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t revision;
    uint32_t rsdt_base;
};

struct __attribute__((__packed__)) __xsdp {
    uint8_t signature[8]; // 'RSD PTR '
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t revision;
    uint32_t rsdt_base;
    uint32_t length;
    uint64_t xsdt_base;
    uint8_t echecksum;
    uint8_t reserved[3];
};

struct __attribute__((__packed__)) __sdt_header {
    uint8_t signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
};

struct __attribute__((__packed__)) __sdt {
    SDT_HEADER header;
    uint32_t sdts[];
};

/**
 * Declarations
*/

int32_t __init_acpi(void);