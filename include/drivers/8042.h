/**
 * 8042
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "drivers/ports.h"
#include "kstdlib/errno.h"
#include "kstdlib/stdio.h"

/**
 * Declarations
*/

int32_t __init_ps2(void);
void __enable_ps2_a_port(void);
void __disable_ps2_a_port(void);
void __ps2_write_byte(uint8_t r, uint8_t v);
uint8_t __ps2_read_byte(void);