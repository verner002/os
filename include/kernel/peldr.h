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

void *__parse_image(uint8_t *image);