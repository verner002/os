/**
 * 8237A
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/8237a.h"

/**
 * __mask_dma_channels
*/

void __mask_dma_channels(void) {
    __outb(DMA_MULTICHANNEL_MASK_REGISTER_03, 0x0f);
    __outb(DMA_MULTICHANNEL_MASK_REGISTER_47, 0x0f);
}

/**
 * __unmask_dma_channels
*/

void __unmask_dma_channels(void) {
    __outb(DMA_MULTICHANNEL_MASK_REGISTER_03, 0x00);
    __outb(DMA_MULTICHANNEL_MASK_REGISTER_47, 0x00);
}

/**
 * __mask_dma_channel_multichannel
*/

/**
 * __unmask_dma_channel_multichannel
*/

/**
 * __active_master_dma_controller
 * 
 * | DAKP | DRWP | EXTW | PRIOR | KOMP | COND | ADH0 | SPSP |
 * |------|------|------|-------|------|------|------|------|
 * |    0 |    0 |    0 |     1 |    0 |    0 |    0 |    0 |
*/

void __active_master_dma_controller(void) {
    __outb(DMA_COMMAND_REGISTER_47, 0x10);
}

/**
 * __deactive_master_dma_controller
 * 
 * | DAKP | DRWP | EXTW | PRIOR | KOMP | COND | ADH0 | SPSP |
 * |------|------|------|-------|------|------|------|------|
 * |    0 |    0 |    0 |     1 |    0 |    1 |    0 |    0 |
*/

void __deactive_master_dma_controller(void) {
    __outb(DMA_COMMAND_REGISTER_47, 0x14);
}

/**
 * __active_slave_dma_controller
 * 
 * | DAKP | DRWP | EXTW | PRIOR | KOMP | COND | ADH0 | SPSP |
 * |------|------|------|-------|------|------|------|------|
 * |    0 |    0 |    0 |     1 |    0 |    0 |    0 |    0 |
*/

void __active_slave_dma_controller(void) {
    __outb(DMA_COMMAND_REGISTER_03, 0x10);
}

/**
 * __deactive_slave_dma_controller
 * 
 * | DAKP | DRWP | EXTW | PRIOR | KOMP | COND | ADH0 | SPSP |
 * |------|------|------|-------|------|------|------|------|
 * |    0 |    0 |    0 |     1 |    0 |    1 |    0 |    0 |
*/

void __deactive_slave_dma_controller(void) {
    __outb(DMA_COMMAND_REGISTER_03, 0x14);
}

/**
 * __wait_for_dma
*/

void __wait_for_dma(void) {
    __outb(POST_CODE_REGISTER, 0x00); // dummy write
}