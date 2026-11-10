/**
 * @file ctxsw.h
 * @author verner002
 * @date 04/09/2026
*/

#pragma once

#include <kernel/types.h>

void ctxsw(unsigned long *old_stack, unsigned long *new_stack, uint32_t cr3);