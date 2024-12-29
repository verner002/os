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
    unsigned int id;
    unsigned int eip;
    unsigned int esp;
    unsigned int ebp;
    // TODO: paging directory
    unsigned int kernel_stack;
    TASK *next;
};