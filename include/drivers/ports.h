/**
 * Ports
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"

/**
 * Constants
*/

#define DMA_START_ADDRESS_REGISTER_0            0x0000 // unusable
#define DMA_COUNT_REGISTER_0                    0x0001 // unusable
#define DMA_START_ADDRESS_REGISTER_1            0x0002
#define DMA_COUNT_REGISTER_1                    0x0003
#define DMA_START_ADDRESS_REGISTER_3            0x0006
#define DMA_COUNT_REGISTER_3                    0x0007
#define DMA_STATUS_REGISTER_03                  0x0008 // read
#define DMA_REQUEST_REGISTER_03                 0x0009
#define DMA_INTERMEDIATE_REGISTER_03            0x000d // read
#define DMA_MASTER_RESET_REGISTER_03            0x000d // write
#define DMA_MASK_RESET_REGISTER_03              0x000e
#define PPI_PORT_B                              0x0061
#define CMOS_ADDRESS_REGISTER                   0x0070
#define CMOS_SELECTED_REGISTER                  0x0071
#define POST_CODE_REGISTER                      0x0080
#define DMA_CHANNEL_3_PAGE_ADDRESS_REGISTER     0x0082
#define DMA_CHANNEL_1_PAGE_ADDRESS_REGISTER     0x0083
#define DMA_CHANNEL_0_PAGE_ADDRESS_REGISTER     0x0087 // unusable
#define DMA_CHANNEL_6_PAGE_ADDRESS_REGISTER     0x0089
#define DMA_CHANNEL_7_PAGE_ADDRESS_REGISTER     0x008a
#define DMA_CHANNEL_5_PAGE_ADDRESS_REGISTER     0x008b
#define DMA_CHANNEL_4_PAGE_ADDRESS_REGISTER     0x008f // unusable
#define DMA_START_ADDRESS_REGISTER_4            0x00c0
#define DMA_COUNT_REGISTER_4                    0x00c2
#define DMA_START_ADDRESS_REGISTER_5            0x00c4
#define DMA_COUNT_REGISTER_5                    0x00c6
#define DMA_START_ADDRESS_REGISTER_6            0x00c8
#define DMA_COUNT_REGISTER_6                    0x00ca
#define DMA_START_ADDRESS_REGISTER_7            0x00cc
#define DMA_COUNT_REGISTER_7                    0x00ce
#define DMA_STATUS_REGISTER_47                  0x00d0 // read
#define DMA_REQUEST_REGISTER_47                 0x00d2
#define DMA_SINGLE_CHANNEL_MASK_REGISTER_47     0x00d4
#define DMA_MODE_REGISTER_47                    0x00d6
#define DMA_FLIP_FLOP_RESET_REGISTER_47         0x00d8
#define DMA_INTERMEDIATE_REGISTER_47            0x00da // read
#define DMA_MASTER_RESET_REGISTER_47            0x00da // write
#define DMA_MASK_RESET_REGISTER_47              0x00dc
#define PCI_CONFIG_ADDRESS                      0x0cf8
#define PCI_CONFIG_DATA                         0x0cfc

/**
 * Declarations
*/

uint8_t __inb(uint16_t r);
void __outb(uint16_t r, uint8_t v);
uint16_t __inw(uint16_t r);
void __outw(uint16_t r, uint16_t v);
uint32_t __ind(uint16_t r);
void __outd(uint16_t r, uint32_t v);