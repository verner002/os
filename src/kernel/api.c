/**
 * Application Programming Interface
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/api.h"

/**
 * Types Definitions
*/

typedef struct __export_function EXPORT_FUNCTION;

/**
 * Structures
*/

struct __export_function {
    const char *name;
    uint32_t address;
};

/**
 * Static Global Variables
*/

static EXPORT_FUNCTION export_table[] = {
    (EXPORT_FUNCTION){
        .name = "__read_file",
        .address = 0xdeadbeef
    }
};

/**
 * __resolve_kernel_api_func
*/

void *__resolve_kernel_api_func(const char *name) {
    const uint32_t exports_count = sizeof(export_table) / sizeof(EXPORT_FUNCTION);
    
    for (uint32_t i = 0; i < exports_count; ++i) {
        EXPORT_FUNCTION *export = &export_table[i];

        if (!strcmp(export->name, name))
            return (void *)export->address;
    }

    return NULL;
}