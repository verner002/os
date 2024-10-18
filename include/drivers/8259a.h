/**
 * 8259a
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "drivers/ports.h"

/**
 * Declarations
*/

void __init_pics(byte master_vec_offset, byte slave_vec_offset);