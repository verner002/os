/**
 * @file mount.h
 * @author verner002
 * @date 03/10/2025
*/

#pragma once

#include "types.h"
#include "kernel/kdev.h"

int mount(kdev_t dev, char const *mountpoint);