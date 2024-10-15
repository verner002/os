/**
 * CMOS
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "types.h"
#include "drivers/cpu.h"
#include "drivers/ports.h"

/**
 * Declarations
*/

void __write_cmos_register(byte r, byte v);
byte __read_cmos_register(byte r);