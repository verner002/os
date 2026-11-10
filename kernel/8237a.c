/**
 * @file 8237a.c
 * @author verner002
 * @date 05/11/2025
*/

#include <kernel/8237a.h>
#include <kernel/io.h>

#define DMA_COMMAND_REGISTER_03                 0x0008 // write
#define DMA_MULTICHANNEL_MASK_REGISTER_03       0x000f
#define DMA_COMMAND_REGISTER_47                 0x00d0 // write
#define DMA_MULTICHANNEL_MASK_REGISTER_47       0x00de

void dma_mask_channels(void) {
    outb(DMA_MULTICHANNEL_MASK_REGISTER_03, 0x0f);
    outb(DMA_MULTICHANNEL_MASK_REGISTER_47, 0x0f);
}

void dma_unmask_channels(void) {
    outb(DMA_MULTICHANNEL_MASK_REGISTER_03, 0x00);
    outb(DMA_MULTICHANNEL_MASK_REGISTER_47, 0x00);
}

void dma_active_master_controller(void) {
    /**
     * | DAKP | DRWP | EXTW | PRIOR | KOMP | COND | ADH0 | SPSP |
     * |------|------|------|-------|------|------|------|------|
     * |    0 |    0 |    0 |     1 |    0 |    0 |    0 |    0 |
    */
    outb(DMA_COMMAND_REGISTER_47, 0x10);
}

void dma_deactive_master_controller(void) {
    /**
     * | DAKP | DRWP | EXTW | PRIOR | KOMP | COND | ADH0 | SPSP |
     * |------|------|------|-------|------|------|------|------|
     * |    0 |    0 |    0 |     1 |    0 |    1 |    0 |    0 |
    */
    outb(DMA_COMMAND_REGISTER_47, 0x14);
}

void dma_active_slave_controller(void) {
    /**
     * | DAKP | DRWP | EXTW | PRIOR | KOMP | COND | ADH0 | SPSP |
     * |------|------|------|-------|------|------|------|------|
     * |    0 |    0 |    0 |     1 |    0 |    0 |    0 |    0 |
    */
    outb(DMA_COMMAND_REGISTER_03, 0x10);
}

void dma_deactive_slave_controller(void) {
    /**
     * | DAKP | DRWP | EXTW | PRIOR | KOMP | COND | ADH0 | SPSP |
     * |------|------|------|-------|------|------|------|------|
     * |    0 |    0 |    0 |     1 |    0 |    1 |    0 |    0 |
    */
    outb(DMA_COMMAND_REGISTER_03, 0x14);
}