/**
 * Filesystem
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"
#include "bool.h"

/**
 * Types Definitions
*/

typedef struct _inode INODE;

/**
 * Structures
*/

struct _inode {
    bool leaf;
    dword minimumDegree;
    dword numberOfKeys;
    dword *keys;
    INODE **children;
    INODE *next;
};