/**
 * 8042
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"
#include "bool.h"
#include "drivers/ports.h"
#include "kstdlib/errno.h"
#include "kstdlib/stdio.h"

/**
 * Constants
*/

// commands
#define PS2_SET_LEDS 0xed
#define PS2_ECHO 0xee
#define PS2_SCANCODE_SET 0xf0
#define PS2_IDENTIFY 0xf2
#define PS2_ENABLE_SCANNING 0xf4
#define PS2_DISABLE_SCANNING 0xf5
#define PS2_RESET 0xff

// responses
#define PS2_SELF_TEST_OK 0xaa
#define PS2_ACK 0xfa
#define PS2_SELF_TEST_FAIL1 0xfc
#define PS2_SELF_TEST_FAIL2 0xfd
#define PS2_RESEND 0xfe

// leds sub-commands
#define LEDS_MASK 7
#define LED_RESET 0
#define LED_SCROLL_LOCK 1
#define LED_NUMBER_LOCK 2
#define LED_CAPS_LOCK 4

// scancode sets sub-commands
#define PS2_SCANCODE_SETS_COUNT 3
#define PS2_GET_SCANCODE_SET 0
#define PS2_SET_SCANCODE_SET_1 1
#define PS2_SET_SCANCODE_SET_2 2
#define PS2_SET_SCANCODE_SET_3 3

#define SCANCODE_SET_1 0
#define SCANCODE_SET_2 1
#define SCANCODE_SET_3 2

#define TABLE_SIZE 132

#define F1_SCANCODE 0x05
#define F2_SCANCODE 0x06
#define F3_SCANCODE 0x04
#define F4_SCANCODE 0x0c
#define F5_SCANCODE 0x03
#define F6_SCANCODE 0x0b
#define F7_SCANCODE 0x83
#define F8_SCANCODE 0x0a
#define F9_SCANCODE 0x01
#define F10_SCANCODE 0x09
#define F11_SCANCODE 0x78
#define F12_SCANCODE 0x07
#define L_ALT_SCANCODE 0x11
#define L_SHIFT_SCANCODE 0x12
#define L_CTRL_SCANCODE 0x14
#define CAPSLOCK_SCANCODE 0x58
#define R_SHIFT_SCANCODE 0x59
#define NUMBERLOCK_SCANCODE 0x77
#define SCROLL_LOCK 0x7e

/**
 * Types Definitions
*/

typedef enum __ps2_device_type PS2_DEVICE_TYPE;
typedef struct __ps2_id PS2_ID;
typedef struct __ps2_device PS2_DEVICE;
typedef struct __scancode_set SCANCODE_SET;

/**
 * Enumerations
*/

enum __ps2_device_type {
    PS2_DEVICE_KEYBOARD = 0,
    PS2_DEVICE_MOUSE
};

/**
 * Structures
*/

struct __ps2_id {
    uint8_t bytes[2];
    uint32_t length;
};

struct __ps2_device {
    PS2_ID id;
    char const *name;
    PS2_DEVICE_TYPE type;
};

struct __scancode_set {
    char const *sname;
    uint32_t table[TABLE_SIZE];
};

/**
 * Global Variables
*/

extern uint32_t
    table_normal[],
    table_shift[],
    table_extended[];

/**
 * Declarations
*/

int32_t __init_ps2(void);
void __set_scancode_handler(uint32_t *table, uint32_t scancode, uint32_t handler);
void __enable_ps2_a_port(void);
void __disable_ps2_a_port(void);
void __ps2_write_byte(uint8_t r, uint8_t v);
uint8_t __ps2_read_byte(void);
void __ps2_update_leds(void);
void __ps2_set_leds(uint8_t l);
void __ps2_echo(void);
int8_t __ps2_get_scancode_set(void);
void __ps2_set_scancode_set(uint8_t scancode_set);
PS2_ID *__ps2_identify(void);
void __ps2_enable_scanning(void);
void __ps2_disable_scanning(void);
int32_t __ps2_reset(void);