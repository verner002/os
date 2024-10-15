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
 * __init_dma_controllers
*/

void __init_dma_controllers(void) {
    asm (
        "mov al, 0x14\n\t" // deactivate controller
        "out 0x08, al\n\t"
        "out 0xd0, al\n\t"
        "out 0x0d, al\n\t" // master clear, mask on
        "out 0xda, al\n\t" // master clear, mask on
        "mov al, 0xc0\n\t" // cascade channel 4
        "out 0xd6, al\n\t" // channel 4 is used for cascade
        "mov al, 0x10\n\t" // activate controller
        "out 0xd0, al\n\t"
        "out 0x08, al"
    );
}

/**
 * __mask_dma_channels
*/

void __mask_dma_channels(void) {
    asm (
        "mov al, 0x0f\n\t"
        "out 0x0f, al\n\t"
        "out 0xde, al"
    );
}

/**
 * __unmask_dma_channels
*/

void __unmask_dma_channels(void) {
    asm (
        "xor al, al\n\t"
        "out 0x0f, al\n\t"
        "out 0xde, al"
    );
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
    asm (
        "mov al, 0x10\n\t"
        "out 0xd0, al"
    );
}

/**
 * __deactive_master_dma_controller
 * 
 * | DAKP | DRWP | EXTW | PRIOR | KOMP | COND | ADH0 | SPSP |
 * |------|------|------|-------|------|------|------|------|
 * |    0 |    0 |    0 |     1 |    0 |    1 |    0 |    0 |
*/

void __deactive_master_dma_controller(void) {
    asm (
        "mov al, 0x14\n\t"
        "out 0xd0, al"
    );
}

/**
 * __active_slave_dma_controller
 * 
 * | DAKP | DRWP | EXTW | PRIOR | KOMP | COND | ADH0 | SPSP |
 * |------|------|------|-------|------|------|------|------|
 * |    0 |    0 |    0 |     1 |    0 |    0 |    0 |    0 |
*/

void __active_slave_dma_controller(void) {
    asm (
        "mov al, 0x10\n\t"
        "out 0x08, al"
    );
}

/**
 * __deactive_slave_dma_controller
 * 
 * | DAKP | DRWP | EXTW | PRIOR | KOMP | COND | ADH0 | SPSP |
 * |------|------|------|-------|------|------|------|------|
 * |    0 |    0 |    0 |     1 |    0 |    1 |    0 |    0 |
*/

void __deactive_slave_dma_controller(void) {
    asm (
        "mov al, 0x14\n\t"
        "out 0x08, al"
    );
}

/**
 * __wait_for_dma_controllers
*/

void __wait_for_dma_controllers(void) {
    asm (
        "xor al, al\n\t"
        "out 0x80, al" // dummy write
    );
}