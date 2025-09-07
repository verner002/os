/**
 * CMOS
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"
#include "drivers/cpu.h"
#include "drivers/ports.h"

/**
 * Constants
*/

#define CMOS_FLOPPY_TYPE_REGISTER 0x10

/**
 * Declarations
*/

void __write_cmos_register(uint8_t r, uint8_t v);
uint8_t __read_cmos_register(uint8_t r);
void __wait_for_cmos(void);