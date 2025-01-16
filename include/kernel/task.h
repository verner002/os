/**
 * Task
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

typedef struct _task TASK;

/**
 * Structures
*/

struct _task {
    uint32_t id;
    uint32_t eip;
    uint32_t esp;
    uint32_t ebp;
    // TODO: paging directory
    uint32_t kernel_stack;
    TASK *next;
};