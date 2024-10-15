/**
 * 8237A
 * 
 * Author: verner002
*/

#pragma once

/**
 * Declarations
*/

void __init_dma_controllers(void);
void __mask_dma_channels(void);
void __unmask_dma_channels(void);
void __active_master_dma_controller(void);
void __deactive_master_dma_controller(void);
void __active_slave_dma_controller(void);
void __deactive_slave_dma_controller(void);
void __wait_for_dma_controllers(void);