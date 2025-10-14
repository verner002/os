/**
 * @file tty.c
 * @author verner002
 * @date 02/10/2025
*/

#pragma once

#include "types.h"

extern bool __tty0_mutex;

int32_t __tty_init(void);
int32_t __tty_put(char const *data);