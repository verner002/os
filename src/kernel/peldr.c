/**
 * PE Loader
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/peldr.h"

#define PE_MZ_H_MAGIC 0x5a4d
#define PE_PE_H_MAGIC 0x00004550
#define PE_OPT_H_MAGIC 0x010b

typedef enum __pe_machine PE_MACHINE;
typedef enum __pe_error PE_ERROR;
typedef struct __mz_header MZ_HEADER; 
typedef struct __coff_header COFF_HEADER;
typedef struct __opt_header OPT_HEADER;
typedef struct __pe_header PE_HEADER;
typedef struct __section SECTION;
typedef struct __symbol SYMBOL;

enum __pe_machine {
    PE_MACHINE_I386 = 0x014c
};

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

static PE_ERROR pe_error = 0;

/**
 * __parse_
*/

void *__parse_image(uint8_t *image) {
    MZ_HEADER *_mz_h = (MZ_HEADER *)image;

    if (_mz_h->magic != PE_MZ_H_MAGIC) {
        pe_error = PE_ERROR_INVALID_MZ_MAGIC;
        return NULL;
    }

    PE_HEADER *_pe_h = (PE_HEADER *)_mz_h->e_lfanew;

    if (_pe_h->pe_s.magic != PE_PE_H_MAGIC) {
        pe_error = PE_ERROR_INVALID_PE_MAGIC;
        return NULL;
    }

    COFF_HEADER *_coff_h = &_pe_h->coff_h;
    
    if (_coff_h->machine_type != PE_MACHINE_I386) {
        pe_error = PE_ERROR_UNSUPPORTED_MACHINE;
        return NULL;
    }

    OPT_HEADER *_opt_h = &_pe_h->opt_h;

    if (_opt_h->magic != PE_OPT_H_MAGIC) {
        pe_error = PE_ERROR_INVALID_OPT_MAGIC;
        return NULL;
    }

    uint32_t
        image_base = _opt_h->image_base,
        sections_count = _coff_h->sections_count,
        symbol_table = _coff_h->symbol_table_ptr,
        symbols_count = _coff_h->symbols_count,
        string_table = symbol_table + symbols_count * sizeof(SYMBOL);

    SECTION *section_table =
        (SECTION *)(_pe_h + sizeof(_pe_h->pe_s) + sizeof(_coff_h) + _coff_h->opt_h_size);

    for (uint32_t i = 0; i < sections_count; ++i) {
        SECTION *section = &section_table[i];

        // TODO: implement support for gcc long names
        uint8_t *first_char = section->name;

        if (first_char != 0 && first_char != '.') {
            pe_error = PE_ERROR_UNSUPPORTED_LONG_NAME;
            return NULL;
        }

        uint8_t *section_name = section->zeroes ?
            section->name : (uint8_t *)(string_table + section->offset);

        ++section_name;

        // TODO: use hash table
        if (
            !strncmp(section_name, "comment", 7) ||
            !strncmp(section_name, "edata\0\0", 7) ||
            !strncmp(section_name, "idata\0\0", 7) ||
            !strncmp(section_name, "rsrc\0\0\0", 7) ||
            !strncmp(section_name, "tls\0\0\0\0", 7)
        )
            continue;

        // RFC: make sure that size is page aligned?
        uint32_t virtual_size = section->virtual_size;

        if (!virtual_size)
            continue;

        // RFC: make sure that address is page aligned?
        uint32_t virtual_address = image_base + section->virtual_address;

        uint32_t pages_count = (virtual_size + 4095) / 4096;

        void *page = pgalloc(PAGE_NO_MAP);

        if (page) {
            pe_error = PE_ERROR_FAILED_TO_ALLOC_MEM;
            return NULL;
        }

        if (
            __map_page(
                virtual_address,
                (uint32_t)page,
                PAGE_READ_WRITE | PAGE_USER
            )
        ) {
            pe_error = PE_ERROR_COULD_NOT_MAP_PAGE;
            return NULL;
        }

        uint32_t raw_data_size = section->raw_data_size;

        if (raw_data_size)
            memcpy((void *)virtual_address, (void *)section->raw_data_ptr, section->raw_data_size);
        else
            memset((void *)virtual_address, 0, virtual_size);
    }

    return (void *)image_base;
}