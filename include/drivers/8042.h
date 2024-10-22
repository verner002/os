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

/**
 * Declarations
*/

int __init_ps2(void);
void __enable_ps2_a_port(void);
void __disable_ps2_a_port(void);
void __wait_for_ps2_input_buff(void);
void __wait_for_ps2_ouput_buff(void);