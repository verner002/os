/**
 * B+Tree File System
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"
#include "bool.h"
#include "macros.h"
#include "kstdlib/stdlib.h"

/**
 * Types Definitions
*/

typedef struct _inode INODE;
typedef struct _btree BTREE;

/**
 * Structures
*/

struct _inode {
    bool isLeaf;
    dword minimumDegree;
    dword numberOfKeys;
    dword *keys;
    INODE **children;
    INODE *next;
};

struct _btree {
    dword minimumDegree;
    INODE *root;
};