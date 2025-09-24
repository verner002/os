/**
 * PE Loader
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "types.h"
#include "mm/e820.h"
#include "mm/vmm.h"
#include "kstdlib/string.h"

/**
 * Types Definitions
*/

typedef enum __pe_machine PE_MACHINE;
typedef enum __pe_error PE_ERROR;
typedef struct __mz_header MZ_HEADER; 
typedef struct __coff_header COFF_HEADER;
typedef struct __opt_header OPT_HEADER;
typedef struct __pe_header PE_HEADER;
typedef struct __section SECTION;
typedef struct __symbol SYMBOL;

/**
 * Constants
*/

#define PE_MZ_H_MAGIC 0x5a4d
#define PE_PE_H_MAGIC 0x00004550
#define PE_OPT_H_MAGIC 0x010b

/**
 * Enumerations
*/

enum __pe_machine {
    PE_MACHINE_I386 = 0x014c
};

enum __pe_error {
    PE_ERROR_OK = 0,
    PE_ERROR_INVALID_MZ_MAGIC,
    PE_ERROR_INVALID_PE_MAGIC,
    PE_ERROR_UNSUPPORTED_MACHINE,
    PE_ERROR_INVALID_OPT_MAGIC,
    PE_ERROR_UNSUPPORTED_LONG_NAME,
    PE_ERROR_FAILED_TO_ALLOC_MEM,
    PE_ERROR_COULD_NOT_MAP_PAGE
};

/**
 * Structures
*/

struct __mz_header {
    uint16_t magic;
    uint8_t reserved;
    uint32_t e_lfanew;
};

struct __coff_header {
    uint16_t machine_type;
    uint16_t sections_count;
    uint32_t time_date_stamp;
    uint32_t symbol_table_ptr;
    uint32_t symbols_count;
    uint16_t opt_h_size;
    uint16_t characteristics;
};

struct __opt_header {
    uint16_t magic;
    uint8_t major_link_version;
    uint8_t minor_link_version;
    uint32_t text_size;
    uint32_t data_size;
    uint32_t bss_size;
    uint32_t entry_point_ptr;
    uint32_t text_base;
    uint32_t data_base;
    uint32_t image_base;
    uint32_t section_alignment;
    uint32_t file_alignment;
    uint16_t major_os_version;
    uint16_t minor_os_version;
    uint16_t major_image_version;
    uint16_t minor_image_version;
    uint16_t major_subsystem_version;
    uint16_t minor_subsystem_version;
    uint32_t image_size;
    uint32_t headers_size;
    uint32_t checksum;
    uint16_t subsystem;
    uint16_t dll_characteristics;
    uint32_t stack_reserve_size;
    uint32_t stack_commit_size;
    uint32_t heap_reserve_size;
    uint32_t heap_commit_size;
    uint32_t loader_flags;
    uint32_t rva_and_sizes_count;
};

struct __pe_header {
    struct {
        uint32_t magic;
    } pe_s;
    COFF_HEADER coff_h;
    OPT_HEADER opt_h;
};

struct __section {
    union {
        uint8_t name[8];
        struct {
            uint32_t zeroes;
            uint32_t offset;
        };
    };
    uint32_t virtual_size;
    uint32_t virtual_address;
    uint32_t raw_data_size;
    uint32_t raw_data_ptr;
    uint32_t relocations_ptr;
    uint32_t line_numbers_ptr;
    uint16_t relocations_count;
    uint16_t line_numbers_count;
    uint32_t characteristics;
};

struct __symbol {
    uint8_t name[8];
    uint32_t value;
    uint16_t section_number;
    uint16_t type;
    uint8_t storage_class;
    uint8_t auxiliary_count;
};