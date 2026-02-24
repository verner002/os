#pragma once

#include "types.h"
#include "bool.h"

typedef struct __file FILE;

struct __file {
    char *__base; // buffer base
    char *__ptr; // read ptr
    uint32_t __index; // write index
    uint32_t __count; // chars in buffer
    uint32_t __flags; // flags
    uint32_t __size; // buffer size
    char *__fname; // filename
    bool __lock; // mutex
    bool __ready; // ready flag
};