/**
 * @file e820.h
 * @author verner002
 * @date 19/07/2026
*/

#pragma once

#include <kernel/types.h>

struct e820_entry {
    uint32_t base;
    uint32_t length;
    uint32_t type;
};