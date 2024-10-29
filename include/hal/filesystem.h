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

void __read_sector(byte id, byte lba, byte *buffer);
void __write_sector(byte id, byte lba, byte *buffer);