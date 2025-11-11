/**
 * Driver
 * 
 * Author: verner002
*/

#pragma once

#include "null.h"
#include "types.h"
#include "kernel/kobj.h"

int32_t __init_drivers(void);
struct __driver *__register_driver(char const *name, uint8_t major, struct __kobj_type *ktype);