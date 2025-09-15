/**
 * @file ide.h
 * @author verner002
 * @date 04/09/2025
*/

#pragma once

#include "drivers/pci.h"

int32_t __ide_read_blocks(uint8_t drive, uint32_t lba, uint8_t count, uint8_t *buffer);