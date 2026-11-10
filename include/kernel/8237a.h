/**
 * @file 8237a.h
 * @author verner002
 * @date 05/11/2025
*/

#pragma once

void dma_init(void);
void dma_mask_channels(void);
void dma_unmask_channels(void);
void dma_active_master_controller(void);
void dma_deactive_master_controller(void);
void dma_active_slave_controller(void);
void dma_deactive_slave_controller(void);