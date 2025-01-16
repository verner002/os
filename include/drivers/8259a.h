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

void __init_pics(uint8_t master_vec_offset, uint8_t slave_vec_offset);
void __send_eoi_slave(void);
void __send_eoi_master(void);
void __send_eoi(uint8_t irq_number);
void __enable_irqs(void);
void __disable_irqs(void);
void __enable_irq(uint8_t irq_number);
void __disable_irq(uint8_t irq_number);