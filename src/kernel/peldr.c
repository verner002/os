/**
 * PE Loader
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/peldr.h"

/**
 * Static Global Variables
*/

static PE_ERROR pe_error = 0;

/**
 * __parse_
*/

void *__parse_image(uint8_t *image, uint32_t address) {
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

        void *page = e820_rmalloc(4096, TRUE);

        if (page) {
            pe_error = PE_ERROR_FAILED_TO_ALLOC_MEM;
            return NULL;
        }

        if (
            __map_page(
                virtual_address,
                (uint32_t)page,
                PAGE_NONE
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

        return (void *)image_base;
    }
}