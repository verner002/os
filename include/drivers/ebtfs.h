/**
 * Extent B+Tree File System
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"

/**
 * Types Definitions
*/

typedef struct _ebtfs_header EBTFS_HEADER;
typedef struct _ebtfs_block_group_descriptor EBTFS_BLOCK_GROUP_DESCRIPTOR;

/**
 * Enumerations
*/

enum _ebtfs_h_state {
    EBTFS_H_STATE_CLEAN = 0x00,
    EBTFS_H_STATE_ERROR = 0x01
};

enum _ebtfs_h_errors {
    EBTFS_H_ERRORS_CONTINUE = 0x00,
    EBTFS_H_ERRORS_RO = 0x01,
    EBTFS_H_ERRORS_PANIC = 0xff
};

/**
 * Structures
*/

struct _ebtfs_header {
    dword h_magic;
    byte h_state;
    byte h_errors;
    byte h_log_block_size;
    dword h_first_data_block;
    dword h_blocks_count;
    dword h_inodes_count;
    dword h_free_blocks_count;
    dword h_free_inodes_count;
    dword h_blocks_per_group;
    dword h_inodes_per_group;
};

struct _ebtfs_block_group_descriptor {
    dword g_block_bitmap;
    dword g_inode_bitmap;
    dword g_inode_table;
};