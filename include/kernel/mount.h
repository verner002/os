/**
 * @file mount.h
 * @author verner002
 * @date 03/10/2025
*/

#pragma once

#include "types.h"
#include "kernel/kdev.h"

int mount(char const *device, char const *mountpoint);
char *lookup_mountpoint(kdev_t kdev);