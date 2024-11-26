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
void __send_eoi(byte irq_number);
void __enable_irqs(void);
void __disable_irqs(void);
void __enable_irq(byte irq_number);
void __disable_irq(byte irq_number);