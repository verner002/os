/**
 * @file mount.h
 * @author verner002
 * @file 02/09/2025
*/

#pragma once

#include "types.h"
#include "kernel/kdev.h"
#include "hal/vfs.h"

struct __super_block {
    __kdev_t s_dev;
    uint32_t s_block_size; // words per block
    uint32_t s_block_bits; // bits per word
    struct __dentry *s_mounted; // directory entry
    struct __super_block_ops;
};

struct __super_block_ops {
    int32_t (* __read_sblock)(struct __super_block *sblock);
    int32_t (* __sync_sblock)(struct __super_block *sblock);
};