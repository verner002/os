/**
 * @file super.h
 * @author verner002
 * @date 11/09/2025
*/

#pragma once

#include "types.h"
#include "bool.h"

struct __superblock {
    uint32_t s_block_sz; // bytes per block
    uint64_t s_block_cnt; // total number of blocks
    bool s_lock;
    bool s_readonly;
};