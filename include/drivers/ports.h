/**
 * Ports
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "types.h"

#define CMOS_ADDRESS_REGISTER   0x0070
#define CMOS_SELECTED_REGISTER  0x0071
#define POST_CODE_REGISTER      0x0080

/**
 * Declarations
*/

void __outb(word r, byte v);
byte __inb(word r);