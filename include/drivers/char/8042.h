/**
 * @file 8042.h
 * @author verner002
 * @date 03/06/2026
*/

#pragma once

#include "kernel/types.h"

#define PS2_DEV_ID_MAX_LENGTH       2

// scancode sets
#define PS2_SCANCODE_SET_1          1
#define PS2_SCANCODE_SET_2          2
#define PS2_SCANCODE_SET_3          3

typedef enum {
    PS2_DEVICE_TYPE_UNKNOWN,
    PS2_DEVICE_TYPE_KEYBOARD,
    PS2_DEVICE_TYPE_MOUSE
} PS2_DEVICE_TYPE;

typedef struct {
    uint8_t bytes[PS2_DEV_ID_MAX_LENGTH];
    int length;
} PS2_DEVICE_ID;

typedef struct {
    PS2_DEVICE_ID id;
    char const *name;
    PS2_DEVICE_TYPE type;
} PS2_DEVICE;

typedef struct {
    char const *name;
    int populated;
    PS2_DEVICE device;
} PS2_PORT;

int ps2_init(void);