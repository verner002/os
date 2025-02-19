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
#include "kstdlib/stdio.h"

/**
 * Declarations
*/

void __init_pics(uint8_t master_vec_offset, uint8_t slave_vec_offset);
void __send_master_eoi(void);
void __send_slave_eoi(void);
void __send_eoi(uint8_t irq_number);
void __enable_irqs(void);
void __disable_irqs(void);
void __enable_irq(uint8_t irq_number);
void __disable_irq(uint8_t irq_number);
uint8_t __read_master_isr(void);
uint8_t __read_slave_isr(void);