/**
 * File System
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"

/**
 * Declarations
*/

void __read_sector(uint8_t id, uint8_t lba, uint8_t *buffer);
void __write_sector(uint8_t id, uint8_t lba, uint8_t *buffer);