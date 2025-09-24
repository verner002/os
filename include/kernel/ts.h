/**
 * @file ts.h
 * @author verner002
 * @date 23/09/2025
*/

#pragma once

#include "bool.h"

void __flush_tss(void);
uint32_t __read_eip(void);
void __exec_kernelmode(uint32_t eip, uint32_t esp, uint32_t ebp, bool *mutex);
void __exec_usermode(uint32_t eip, uint32_t esp, uint32_t ebp);