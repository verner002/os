/**
 * @file exec.c
 * @author verner002
 * @date 04/11/2025
*/

#include <kernel/exec.h>

/**
 * pid 0 and 1 use kernel page tables with u/s = 1
 * so we have to create another page tables mapping
 * kernel pages with u/s = 0 (which will by used by
 * exec* functions when initializing address space)
*/