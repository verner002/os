/**
 * @file 8042.c
 * @author verner002
 * @date 03/06/2026
*/

#include "null.h"

#include "arch/8042.h"
#include "arch/io.h"
#include "arch/vga.h"

// ports
#define PS2_REG_DATA                    0x60
#define PS2_REG_STATUS                  0x64 // read
#define PS2_REG_COMMAND                 0x64 // write

// commands
#define PS2_CMD_READ_CONFIG_WORD        0x20
#define PS2_CMD_WRITE_CONFIG_WORD       0x60
#define PS2_CMD_PORT_B_DISABLE          0xa7
#define PS2_CMD_PORT_B_ENABLE           0xa8
#define PS2_CMD_SELF_TEST               0xaa
#define PS2_CMD_PORT_A_TEST             0xab
#define PS2_CMD_PORT_A_DISABLE          0xad
#define PS2_CMD_PORT_A_ENABLE           0xae
#define PS2_CMD_SET_LEDS                0xed
#define PS2_CMD_ECHO                    0xee
#define PS2_CMD_SCANCODE_SET            0xf0
#define PS2_CMD_IDENTIFY                0xf2
#define PS2_CMD_ENABLE_SCANNING         0xf4
#define PS2_CMD_DISABLE_SCANNING        0xf5
#define PS2_CMD_RESET                   0xff

// command error codes
#define PS2_CMD_ERR_SELF_TEST_OK        0xaa
#define PS2_CMD_ERR_ECHO                0xee
#define PS2_CMD_ERR_ACK                 0xfa
#define PS2_CMD_ERR_SELF_TEST_FAIL1     0xfc
#define PS2_CMD_ERR_SELF_TEST_FAIL2     0xfd
#define PS2_CMD_ERR_RESEND              0xfe

// number of scancode sets
#define PS2_SCANCODE_SETS_COUNT         3

// scancode sets sub-commands
#define PS2_GET_SCANCODE_SET            0
#define PS2_SET_SCANCODE_SET_1          1
#define PS2_SET_SCANCODE_SET_2          2
#define PS2_SET_SCANCODE_SET_3          3

static PS2_PORT
    port_a = (PS2_PORT){
        .populated = 0
    },
    port_b = (PS2_PORT){
        .populated = 0
    };

static PS2_DEVICE known_devices[] = {
    (PS2_DEVICE){
        .id = (PS2_DEVICE_ID){
            .bytes = {},
            .length = 0
        },
        .name = "AT Keyboard",
        .type = PS2_DEVICE_TYPE_KEYBOARD
    },
    (PS2_DEVICE){
        .id = (PS2_DEVICE_ID){
            .bytes = { 0x00 },
            .length = 1
        },
        .name = "Standard PS/2 Mouse",
        .type = PS2_DEVICE_TYPE_MOUSE
    },
    (PS2_DEVICE){
        .id = (PS2_DEVICE_ID){
            .bytes = { 0x03 },
            .length = 1
        },
        .name = "Mouse With Scroll Whell",
        .type = PS2_DEVICE_TYPE_MOUSE
    },
    (PS2_DEVICE){
        .id = (PS2_DEVICE_ID){
            .bytes = { 0x04 },
            .length = 1
        },
        .name = "Mouse With Five Buttons",
        .type = PS2_DEVICE_TYPE_MOUSE
    },
    (PS2_DEVICE){
        .id = (PS2_DEVICE_ID){
            .bytes = { 0xab, 0x83 },
            .length = 2
        },
        .name = "MF2 Keyboard",
        .type = PS2_DEVICE_TYPE_KEYBOARD
    },
    (PS2_DEVICE){
        .id = (PS2_DEVICE_ID){
            .bytes = { 0xab, 0xc1 },
            .length = 2
        },
        .name = "MF2 Keyboard",
        .type = PS2_DEVICE_TYPE_KEYBOARD
    }
};

uint32_t table_normal[] = {
    '\0', // 0x00
    0, // 0x01 - f9
    '\0', // 0x02
    0, // 0x03 - f5
    0, // 0x04 - f3
    0, // 0x05 - f1
    0, // 0x06 - f2
    0, // 0x07 - f12
    '\0', // 0x08
    0, // 0x09 - f10
    0, // 0x0a - f8
    0, // 0x0b - f6
    0, // 0x0c - f4
    '\t', // 0x0d - tab
    '`', // 0x0e - `
    '\0', // 0x0f
    '\0', // 0x10
    0, // 0x11 - left alt
    0, // 0x12 - left shift
    '\0', // 0x13
    0, // 0x14 - left control
    'q', // 0x15 - q
    '1', // 0x16 - 1
    '\0', // 0x17
    '\0', // 0x18
    '\0', // 0x19
    'z', // 0x1a - z
    's', // 0x1b - s
    'a', // 0x1c - a
    'w', // 0x1d - w
    '2', // 0x1e - 2
    '\0', // 0x1f
    '\0', // 0x20
    'c', // 0x21 - c
    'x', // 0x22 - x
    'd', // 0x23 - d
    'e', // 0x24 - e
    '4', // 0x25 - 4
    '3', // 0x26 - 3
    '\0', // 0x27
    '\0', // 0x28
    ' ', // 0x29 - space
    'v', // 0x2a - v
    'f', // 0x2b - f
    't', // 0x2c - t
    'r', // 0x2d - r
    '5', // 0x2e - 5
    '\0', // 0x2f
    '\0', // 0x30
    'n', // 0x31 - n
    'b', // 0x32 - b
    'h', // 0x33 - h
    'g', // 0x34 - g
    'y', // 0x35 - y
    '6', // 0x36 - 6
    '\0', // 0x37
    '\0', // 0x38
    '\0', // 0x39
    'm', // 0x3a - m
    'j', // 0x3b - j
    'u', // 0x3c - u
    '7', // 0x3d - 7
    '8', // 0x3e - 8
    '\0', // 0x3f
    '\0', // 0x40
    ',', // 0x41 - ,
    'k', // 0x42 - k
    'i', // 0x43 - i
    'o', // 0x44 - o
    '0', // 0x45 - 0
    '9', // 0x46 - 9
    '\0', // 0x47
    '\0', // 0x48
    '.', // 0x49 - .
    '/', // 0x4a - /
    'l', // 0x4b - l
    ';', // 0x4c - ;
    'p', // 0x4d - p
    '-', // 0x4e - -
    '\0', // 0x4f
    '\0', // 0x50
    '\0', // 0x51
    '\'', // 0x52 - '
    '\0', // 0x53
    '[', // 0x54 - [
    '=', // 0x55 - =
    '\0', // 0x56
    '\0', // 0x57
    0, // 0x58 - capslock
    0, // 0x59 - right shift
    '\n', // 0x5a - enter
    ']', // 0x5b - ]
    '\0', // 0x5c
    '\\', // 0x5d - backslash
    '\0', // 0x5e
    '\0', // 0x5f
    '\0', // 0x60
    '\0', // 0x61
    '\0', // 0x62
    '\0', // 0x63
    '\0', // 0x64
    '\0', // 0x65
    '\b', // 0x66 - backspace
    '\0', // 0x67
    '\0', // 0x68
    '1', // 0x69 - keypad 1
    '\0', // 0x6a
    '4', // 0x6b - keypad 4
    '7', // 0x6c - keypad 7
    '\0', // 0x6d
    '\0', // 0x6e
    '\0', // 0x6f
    '0', // 0x70 - keypad 0
    '.', // 0x71 - keypad .
    '2', // 0x72 - keypad 2
    '5', // 0x73 - keypad 5
    '6', // 0x74 - keypad 6
    '8', // 0x75 - keypad 8
    '\e', // 0x76 - escape
    0, // 0x77 - numberlock
    0, // 0x78 - f11
    '+', // 0x79 - keypad +
    '3', // 0x7a - keypad 3
    '-', // 0x7b - keypad -
    '*', // 0x7c - keypad *
    '9', // 0x7d - keypad 9
    0, // 0x7e - scrolllock
    '\0', // 0x7f
    '\0', // 0x80
    '\0', // 0x81
    '\0', // 0x82
    0, // 0x83 - f7
};

uint32_t table_shift[] = {
    '\0', // 0x00
    '\0', // 0x01 - f9
    '\0', // 0x02
    '\0', // 0x03 - f5
    '\0', // 0x04 - f3
    '\0', // 0x05 - f1
    '\0', // 0x06 - f2
    '\0', // 0x07 - f12
    '\0', // 0x08
    '\0', // 0x09 - f10
    '\0', // 0x0a - f8
    '\0', // 0x0b - f6
    '\0', // 0x0c - f4
    '\t', // 0x0d - tab
    '~', // 0x0e - `
    '\0', // 0x0f
    '\0', // 0x10
    '\0', // 0x11 - left alt
    '\0', // 0x12 - left shift
    '\0', // 0x13
    '\0', // 0x14 - left control
    'Q', // 0x15 - q
    '!', // 0x16 - 1
    '\0', // 0x17
    '\0', // 0x18
    '\0', // 0x19
    'Z', // 0x1a - z
    'S', // 0x1b - s
    'A', // 0x1c - a
    'W', // 0x1d - w
    '@', // 0x1e - 2
    '\0', // 0x1f
    '\0', // 0x20
    'C', // 0x21 - c
    'X', // 0x22 - x
    'D', // 0x23 - d
    'E', // 0x24 - e
    '$', // 0x25 - 4
    '#', // 0x26 - 3
    '\0', // 0x27
    '\0', // 0x28
    ' ', // 0x29 - space
    'V', // 0x2a - v
    'F', // 0x2b - f
    'T', // 0x2c - t
    'R', // 0x2d - r
    '%', // 0x2e - 5
    '\0', // 0x2f
    '\0', // 0x30
    'N', // 0x31 - n
    'B', // 0x32 - b
    'H', // 0x33 - h
    'G', // 0x34 - g
    'Y', // 0x35 - y
    '^', // 0x36 - 6
    '\0', // 0x37
    '\0', // 0x38
    '\0', // 0x39
    'M', // 0x3a - m
    'J', // 0x3b - j
    'U', // 0x3c - u
    '&', // 0x3d - 7
    '*', // 0x3e - 8
    '\0', // 0x3f
    '\0', // 0x40
    '<', // 0x41 - ,
    'K', // 0x42 - k
    'I', // 0x43 - i
    'O', // 0x44 - o
    ')', // 0x45 - 0
    '(', // 0x46 - 9
    '\0', // 0x47
    '\0', // 0x48
    '>', // 0x49 - .
    '?', // 0x4a - /
    'L', // 0x4b - l
    ':', // 0x4c - ;
    'P', // 0x4d - p
    '_', // 0x4e - -
    '\0', // 0x4f
    '\0', // 0x50
    '\0', // 0x51
    '"', // 0x52 - '
    '\0', // 0x53
    '{', // 0x54 - [
    '+', // 0x55 - =
    '\0', // 0x56
    '\0', // 0x57
    '\0', // 0x58 - capslock
    '\0', // 0x59 - right shift
    '\n', // 0x5a - enter
    '}', // 0x5b - ]
    '\0', // 0x5c
    '|', // 0x5d - backslash
    '\0', // 0x5e
    '\0', // 0x5f
    '\0', // 0x60
    '\0', // 0x61
    '\0', // 0x62
    '\0', // 0x63
    '\0', // 0x64
    '\0', // 0x65
    '\b', // 0x66 - backspace
    '\0', // 0x67
    '\0', // 0x68
    '1', // 0x69 - keypad 1
    '\0', // 0x6a
    '4', // 0x6b - keypad 4
    '7', // 0x6c - keypad 7
    '\0', // 0x6d
    '\0', // 0x6e
    '\0', // 0x6f
    '0', // 0x70 - keypad 0
    '.', // 0x71 - keypad .
    '2', // 0x72 - keypad 2
    '5', // 0x73 - keypad 5
    '6', // 0x74 - keypad 6
    '8', // 0x75 - keypad 8
    '\e', // 0x76 - escape
    '\0', // 0x77 - numberlock
    '\0', // 0x78 - f11
    '+', // 0x79 - keypad +
    '3', // 0x7a - keypad 3
    '-', // 0x7b - keypad -
    '*', // 0x7c - keypad *
    '9', // 0x7d - keypad 9
    '\0', // 0x7e - scrolllock
    '\0', // 0x7f
    '\0', // 0x80
    '\0', // 0x81
    '\0', // 0x82
    '\0', // 0x83 - f7
};

uint32_t table_extended[] = {
    '\0', // 0x00
    '\0', // 0x01 - f9
    '\0', // 0x02
    '\0', // 0x03 - f5
    '\0', // 0x04 - f3
    '\0', // 0x05 - f1
    '\0', // 0x06 - f2
    '\0', // 0x07 - f12
    '\0', // 0x08
    '\0', // 0x09 - f10
    '\0', // 0x0a - f8
    '\0', // 0x0b - f6
    '\0', // 0x0c - f4
    '\0', // 0x0d - tab
    '\0', // 0x0e - `
    '\0', // 0x0f
    '\0', // 0x10
    '\0', // 0x11 - left alt
    '\0', // 0x12 - left shift
    '\0', // 0x13
    '\0', // 0x14 - left control
    '\0', // 0x15 - q
    '\0', // 0x16 - 1
    '\0', // 0x17
    '\0', // 0x18
    '\0', // 0x19
    '\0', // 0x1a - z
    '\0', // 0x1b - s
    '\0', // 0x1c - a
    '\0', // 0x1d - w
    '\0', // 0x1e - 2
    '\0', // 0x1f
    '\0', // 0x20
    '\0', // 0x21 - c
    '\0', // 0x22 - x
    '\0', // 0x23 - d
    '\0', // 0x24 - e
    '\0', // 0x25 - 4
    '\0', // 0x26 - 3
    '\0', // 0x27
    '\0', // 0x28
    '\0', // 0x29 - space
    '\0', // 0x2a - v
    '\0', // 0x2b - f
    '\0', // 0x2c - t
    '\0', // 0x2d - r
    '\0', // 0x2e - 5
    '\0', // 0x2f
    '\0', // 0x30
    '\0', // 0x31 - n
    '\0', // 0x32 - b
    '\0', // 0x33 - h
    '\0', // 0x34 - g
    '\0', // 0x35 - y
    '\0', // 0x36 - 6
    '\0', // 0x37
    '\0', // 0x38
    '\0', // 0x39
    '\0', // 0x3a - m
    '\0', // 0x3b - j
    '\0', // 0x3c - u
    '\0', // 0x3d - 7
    '\0', // 0x3e - 8
    '\0', // 0x3f
    '\0', // 0x40
    '\0', // 0x41 - ,
    '\0', // 0x42 - k
    '\0', // 0x43 - i
    '\0', // 0x44 - o
    '\0', // 0x45 - 0
    '\0', // 0x46 - 9
    '\0', // 0x47
    '\0', // 0x48
    '\0', // 0x49 - .
    '\0', // 0x4a - /
    '\0', // 0x4b - l
    '\0', // 0x4c - ;
    '\0', // 0x4d - p
    '\0', // 0x4e - -
    '\0', // 0x4f
    '\0', // 0x50
    '\0', // 0x51
    '\0', // 0x52 - '
    '\0', // 0x53
    '\0', // 0x54 - [
    '\0', // 0x55 - =
    '\0', // 0x56
    '\0', // 0x57
    '\0', // 0x58 - capslock
    '\0', // 0x59 - right shift
    '\0', // 0x5a - enter
    '\0', // 0x5b - ]
    '\0', // 0x5c
    '\0', // 0x5d - backslash
    '\0', // 0x5e
    '\0', // 0x5f
    '\0', // 0x60
    '\0', // 0x61
    '\0', // 0x62
    '\0', // 0x63
    '\0', // 0x64
    '\0', // 0x65
    '\0', // 0x66 - backspace
    '\0', // 0x67
    '\0', // 0x68
    '\0', // 0x69 - keypad 1
    '\0', // 0x6a
    '\0', // 0x6b - keypad 4
    '\0', // 0x6c - keypad 7
    '\0', // 0x6d
    '\0', // 0x6e
    '\0', // 0x6f
    '\0', // 0x70 - keypad 0
    '\0', // 0x71 - keypad .
    '\0', // 0x72 - keypad 2
    '\0', // 0x73 - keypad 5
    '\0', // 0x74 - keypad 6
    '\0', // 0x75 - keypad 8
    '\0', // 0x76 - escape
    '\0', // 0x77 - numberlock
    '\0', // 0x78 - f11
    '\0', // 0x79 - keypad +
    '\0', // 0x7a - keypad 3
    '\0', // 0x7b - keypad -
    '\0', // 0x7c - keypad *
    '\0', // 0x7d - keypad 9
    '\0', // 0x7e - scrolllock
    '\0', // 0x7f
    '\0', // 0x80
    '\0', // 0x81
    '\0', // 0x82
    '\0', // 0x83 - f7
};

#define PS2_ERR_TIMEDOUT            1
#define PS2_ERR_SELF_TEST           2
#define PS2_ERR_PORT_TEST           3
#define PS2_ERR_UNKNOWN             -1

// internal error indicator
// result of the last internal operation
static int error = 0;

static void ps2_write_byte(uint8_t reg, uint8_t value) {
    for (int i = 0; i < 1000; ++i) {
        if (!(inb(PS2_REG_STATUS) & 0x02)) {
            outb(reg, value);
            return;
        }

        io_wait();
    }

    error = PS2_ERR_TIMEDOUT;
}

static uint8_t ps2_read_byte(void) {
    for (int i = 0; i < 1000; ++i) {
        if (inb(PS2_REG_STATUS) & 0x01)
            return inb(PS2_REG_DATA);

        io_wait();
    }

    error = PS2_ERR_TIMEDOUT;
    return 0;
}

static void ps2_devices_reset(void) {
    for (int i = 0; i < 3; ++i) {
        error = 0;

        // send reset command
        ps2_write_byte(PS2_REG_DATA, PS2_CMD_RESET);

        if (error)
            return;

        // read reset command response
        // expecting 0xfa or 0xfe
        uint8_t response = ps2_read_byte();

        if (error) {
            // NOTE: report port not populated
            return;
        }

        if (response == PS2_CMD_ERR_RESEND) {
            error = PS2_ERR_TIMEDOUT;
            continue;
        }

        if (response != PS2_CMD_ERR_ACK) {
            vga_write("ps2: hardware failure: unexpected response code\n");
            error = PS2_ERR_UNKNOWN;
            return;
        }

        // read self test result
        // expecting 0xaa, 0xfc or 0xfd
        uint8_t result_code = ps2_read_byte();

        if (error)
            return;

        if (result_code == PS2_CMD_ERR_SELF_TEST_FAIL1 || result_code == PS2_CMD_ERR_SELF_TEST_FAIL2) {
            error = PS2_ERR_SELF_TEST;
            return;
        }

        if (result_code != PS2_CMD_ERR_SELF_TEST_OK) {
            vga_write("ps2: hardware failure: unexpected result code\n");
            error =  PS2_ERR_UNKNOWN;
            return;
        }

        PS2_DEVICE_ID id;

        while (1) {
            id.bytes[id.length] = ps2_read_byte();

            // time-out means "all id bytes sent"
            if (error == PS2_ERR_TIMEDOUT)
                break;

            if (error)
                return;

            if (id.length >= PS2_DEV_ID_MAX_LENGTH) {
                error = PS2_ERR_UNKNOWN;
                return;
            }

            ++id.length;
        }

        port_a.device.id = id;
        return;
    }
}

static void ps2_enable_scanning(void) {
    for (int i = 0; i < 3; ++i) {
        error = 0;

        ps2_write_byte(PS2_REG_DATA, PS2_CMD_ENABLE_SCANNING);
    
        if (error)
            return;

        // expecting 0xfa or 0xfe
        uint8_t response = ps2_read_byte();

        if (error)
            return;

        if (response == PS2_CMD_ERR_RESEND) {
            error = PS2_ERR_TIMEDOUT;
            continue;
        }

        if (response != PS2_CMD_ERR_ACK) {
            vga_write("ps2: hardware failure: unexpected response code\n");
            error = PS2_ERR_UNKNOWN;
            return;
        }

        return;
    }
}

static void ps2_disable_scanning(void) {
    for (int i = 0; i < 3; ++i) {
        error = 0;

        ps2_write_byte(PS2_REG_DATA, PS2_CMD_DISABLE_SCANNING);
    
        if (error)
            return;

        // expecting 0xfa or 0xfe
        uint8_t response = ps2_read_byte();

        if (error)
            return;

        if (response == PS2_CMD_ERR_RESEND) {
            error = PS2_ERR_TIMEDOUT;
            continue;
        }

        if (response != PS2_CMD_ERR_ACK) {
            vga_write("ps2: hardware failure: unexpected response code\n");
            error = PS2_ERR_UNKNOWN;
            return;
        }

        return;
    }
}

static void ps2_set_scancode_set(uint8_t scancode_set) {
    if (scancode_set >= PS2_SCANCODE_SETS_COUNT)
        return;

    for (uint32_t i = 0; i < 3; ++i) {
        error = 0;

        ps2_write_byte(PS2_REG_DATA, PS2_CMD_SCANCODE_SET);

        if (error)
            return;

        ps2_write_byte(PS2_REG_DATA, scancode_set);

        if (error)
            return;

        // expecting 0xfa or 0xfe
        uint8_t response = ps2_read_byte();

        if (error)
            return;

        if (response == PS2_CMD_ERR_RESEND) {
            error = PS2_ERR_TIMEDOUT;
            continue;
        }

        if (response != PS2_CMD_ERR_ACK) {
            vga_write("ps2: hardware failure: unexpected response code");
            error = PS2_ERR_UNKNOWN;
            return;
        }

        return;
    }
}

int ps2_init(void) {
    error = 0;

    uint8_t buffer;

    // disable port a
    ps2_write_byte(PS2_REG_COMMAND, PS2_CMD_PORT_A_DISABLE);

    if (error)
        return error;

    // disable port b
    ps2_write_byte(PS2_REG_COMMAND, PS2_CMD_PORT_B_DISABLE);

    if (error)
        return error;

    // flush output buffer
    while (inb(PS2_REG_STATUS) & 0x01) {
        inb(PS2_REG_DATA);
        io_wait();
    }

    // send read config word command
    ps2_write_byte(PS2_REG_COMMAND, PS2_CMD_READ_CONFIG_WORD);

    if (error)
        return error;

    // read config word
    buffer = ps2_read_byte();

    if (error)
        return error;

    // new configuration:
    //  - disable port a translantion
    //  - disable port a and port b irqs
    //  - enable port a clock signal
    uint8_t config_byte = buffer & 0x24;

    // send write config word command
    ps2_write_byte(PS2_REG_COMMAND, PS2_CMD_WRITE_CONFIG_WORD);
    
    if (error)
        return error;

    // write config word
    ps2_write_byte(PS2_REG_DATA, config_byte);
    
    if (error)
        return error;

    // send self test command
    ps2_write_byte(PS2_REG_COMMAND, PS2_CMD_SELF_TEST);
    
    if (error)
        return error;

    // read self test result
    buffer = ps2_read_byte();

    if (buffer != 0x55)
        return PS2_ERR_SELF_TEST;

#ifdef PS2_RESTORE_CONFIG_WORD_AFTER_SELF_TEST
    // send write config word command
    ps2_write_byte(PS2_REG_COMMAND, PS2_CMD_WRITE_CONFIG_WORD);
    
    if (error)
        return error;

    // write config word
    ps2_write_byte(PS2_REG_DATA, config_byte);
#endif
    
    if (error)
        return error;

    // send test port a command
    ps2_write_byte(PS2_REG_COMMAND, PS2_CMD_PORT_A_TEST);
    
    if (error)
        return error;

    // read port a test result
    buffer = ps2_read_byte();
    
    if (error)
        return error;

    if (buffer)
        return PS2_ERR_PORT_TEST;

    // send read config word command
    ps2_write_byte(PS2_REG_COMMAND, PS2_CMD_READ_CONFIG_WORD);

    if (error)
        return error;

    // read config word
    buffer = ps2_read_byte();
    
    if (error)
        return error;

    // new configuration:
    //  - enable port a irq
    config_byte = buffer | 0x01;

    // send write config word command
    ps2_write_byte(PS2_REG_COMMAND, PS2_CMD_WRITE_CONFIG_WORD);

    if (error)
        return error;

    // write config word
    ps2_write_byte(PS2_REG_DATA, config_byte);

    if (error)
        return error;
    
    // enable port a
    ps2_write_byte(PS2_REG_COMMAND, PS2_CMD_PORT_A_ENABLE);

    if (error)
        return error;

    // reset ps2 device(s)
    ps2_devices_reset();

    if (error)
        return error;
        
    // TODO:
    //  - identify device name and type
    //  - set scancode if known keyboard
    //  - reset leds
    return 0;
}

/*void __ps2_update_leds(void) {
    for (uint32_t i = 0; i < 10; ++i) {
        __ps2_write_byte(PS2_REG_DATA, PS2_CMD_SET_LEDS);
        __ps2_write_byte(PS2_REG_DATA, leds);
    
        errno = 0; // reset errno

        uint8_t response = __ps2_read_byte();

        if (errno)
            continue;

        if (response == PS2_CMD_ERR_RESEND)
            continue;

        if (response != PS2_CMD_ERR_ACK) {
            vga_write("ps2: fatal: unknown response code: %u\n expected 0xfa or 0xfe\n", response);
            errno = EIO;
        }

        return;
    }
}

void __ps2_set_leds(uint8_t l) {
    leds = l & LEDS_MASK;
    __ps2_update_leds();
}

void __ps2_echo(void) {
    for (uint32_t i = 0; i < 10; ++i) {
        __ps2_write_byte(PS2_REG_DATA, PS2_CMD_ECHO);
    
        errno = 0; // reset errno

        uint8_t response = __ps2_read_byte();

        if (errno)
            continue;

        if (response == PS2_CMD_ERR_RESEND)
            continue;

        if (response != PS2_CMD_ERR_ECHO) {
            vga_write("ps2: fatal: unknown response code: %u\n expected 0xee\n", response);
            errno = EIO;
        }

        return;
    }
}

int8_t __ps2_get_scancode_set(void) {
    for (uint32_t i = 0; i < 10; ++i) {
        __ps2_write_byte(PS2_REG_DATA, PS2_CMD_SCANCODE_SET);
        __ps2_write_byte(PS2_REG_DATA, PS2_GET_SCANCODE_SET);
    
        errno = 0; // reset errno

        uint8_t response = __ps2_read_byte();

        if (errno)
            continue;

        // FIXME: this might be wrong according to osdev.org/PS/2_Keyboard
        if (response == PS2_RESEND)
            continue;

        if (response != PS2_ACK) {
            vga_write("ps2: fatal: unknown response code: %u\n expected 0xfa or 0xfe\n", response);
            errno = EIO;
            return -1; // invalid
        }

        errno = 0;

        int8_t scancode_set = __ps2_read_byte();

        if (errno)
            continue;

        return scancode_set;
    }

    return -1;
}*/