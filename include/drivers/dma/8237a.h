/**
 * 8237A
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "drivers/ports.h"
#include "drivers/cpu.h"

/**
 * Declarations
*/

void __init_dma(void);
void __mask_dma_channels(void);
void __unmask_dma_channels(void);
void __active_master_dma_controller(void);
void __deactive_master_dma_controller(void);
void __active_slave_dma_controller(void);
void __deactive_slave_dma_controller(void);
void __wait_for_dma(void);