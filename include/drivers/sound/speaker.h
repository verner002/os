/**
 * Speaker
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"
#include "drivers/ports.h"

/**
 * Declarations
*/

void __init_speaker(void);
void __enable_speaker(void);
void __disable_speaker(void);
void __play_note(uint16_t f);