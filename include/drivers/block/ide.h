/**
 * @file ide.h
 * @author verner002
 * @date 04/09/2025
*/

#pragma once

#include "drivers/bus/pci.h"

int32_t ide_read_blocks(uint8_t minor, uint32_t lba, uint32_t count, char *buffer);