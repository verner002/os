/**
 * 8042
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "kstdlib/errno.h"
#include "drivers/ports.h"

/**
 * Declarations
*/

void __init_ps2(void);
void __enable_ps2_a_port(void);
void __disable_ps2_a_port(void);
void __ps2_write_byte(byte r, byte v);
byte __ps2_read_byte(void);