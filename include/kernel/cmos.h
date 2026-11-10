/**
 * @file cmos.h
 * @author verner002
 * @date 23/04/2026
*/

#pragma once

#include "kernel/types.h"

#define CMOS_FLOPPY_TYPE_REGISTER   0x10
//#define CMOS_..._REGISTER (time, etc.)

void cmos_write(uint8_t reg, uint8_t value);
uint8_t cmos_read(uint8_t reg);