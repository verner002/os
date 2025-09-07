/**
 * 8042
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/8042.h"

/**
 * Static Global Variables
*/

static uint8_t leds;
static PS2_DEVICE
    *ps2_port_a,
    *ps2_port_b;

static PS2_DEVICE known_devices[] = {
    (PS2_DEVICE){
        .id = (PS2_ID){
            .bytes = {},
            .length = 0
        },
        .name = "AT Keyboard",
        .type = PS2_DEVICE_KEYBOARD
    },
    (PS2_DEVICE){
        .id = (PS2_ID){
            .bytes = { 0x00 },
            .length = 1
        },
        .name = "Standard PS/2 Mouse",
        .type = PS2_DEVICE_MOUSE
    },
    (PS2_DEVICE){
        .id = (PS2_ID){
            .bytes = { 0x03 },
            .length = 1
        },
        .name = "Mouse With Scroll Whell",
        .type = PS2_DEVICE_MOUSE
    },
    (PS2_DEVICE){
        .id = (PS2_ID){
            .bytes = { 0x04 },
            .length = 1
        },
        .name = "Mouse With Five Buttons",
        .type = PS2_DEVICE_MOUSE
    },
    (PS2_DEVICE){
        .id = (PS2_ID){
            .bytes = { 0xab, 0x83 },
            .length = 2
        },
        .name = "MF2 Keyboard",
        .type = PS2_DEVICE_KEYBOARD
    },
    (PS2_DEVICE){
        .id = (PS2_ID){
            .bytes = { 0xab, 0xc1 },
            .length = 2
        },
        .name = "MF2 Keyboard",
        .type = PS2_DEVICE_KEYBOARD
    }
};

/**
 * Global Variables
*/

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

/**
 * __init_ps2
*/

int32_t __init_ps2(void) {
    printk("\033[33mps2:\033[37m Initializing... ");

    uint8_t buffer;

    __disable_ps2_a_port();
    __ps2_write_byte(PS2_COMMAND_REGISTER, 0xa7); // disable port b, we won't use it
    __inb(PS2_DATA_PORT_REGISTER); // flush output buffer

    __ps2_write_byte(PS2_COMMAND_REGISTER, 0x20); // read config uint8_t

    buffer = __ps2_read_byte();
    if (errno) {
        printf("Error\n");
        return -1;
    }

    uint8_t config_byte = buffer & 0x24; // disable translantion for port a, disable irqs, enable signal for port a

    __ps2_write_byte(PS2_COMMAND_REGISTER, 0x60); // write config uint8_t    
    __ps2_write_byte(PS2_DATA_PORT_REGISTER, config_byte);

    __ps2_write_byte(PS2_COMMAND_REGISTER, 0xaa); // test ps/2 controller

    buffer = __ps2_read_byte();
    if (errno) {
        printf("Error\n");
        return -1;
    }

    if (buffer != 0x55) {
        printf("Error\n");
        return -1;
    }

    __ps2_write_byte(PS2_COMMAND_REGISTER, 0xab); // test port a

    buffer = __ps2_read_byte();
    if (errno) {
        printf("Error\n");
        return -1;
    }

    if (buffer) {
        printf("Error\n");
        return -1;
    }

    __ps2_write_byte(PS2_COMMAND_REGISTER, 0x20); // read config uint8_t

    buffer = __ps2_read_byte();
    if (errno) {
        printf("Error\n");
        return -1;
    }

    config_byte = buffer | 0x01; // enable irq for port a

    __ps2_write_byte(PS2_COMMAND_REGISTER, 0x60); // write config uint8_t
    __ps2_write_byte(PS2_DATA_PORT_REGISTER, config_byte);
    __enable_ps2_a_port();

    printf("Ok\n");

    printk("\033[33mps2:\033[37m port-a: Resetting... ");
    
    if (__ps2_reset()) {
        printf("Error\n");
        return -1;
    }

    printf("Ok\n");
    printk("\033[33mps2:\033[37m port-a: Identifying... ");

    ps2_port_a = NULL;
    ps2_port_b = NULL;

    PS2_ID *id = __ps2_identify();

    // FIXME: check if id is set
    putchar('\n');
    printk("\033[33mps2:\033[37m port-a: Device ID:\n");

    uint8_t *id_bytes = id->bytes;
    uint32_t id_length = id->length;

    // dump device id
    for (uint32_t i = 0; i < id_length; ++i)
        printf("                             - ID[%u]: 0x%02x\n", i, id_bytes[i]);

    printk("\033[33mps2:\033[37m port-a: Device type: ");
    
    for (uint32_t i = 0; i < sizeof(known_devices) / sizeof(PS2_DEVICE); ++i) {
        PS2_DEVICE *device = &known_devices[i];
        PS2_ID *device_id = &device->id;

        if (device_id->length != id_length)
            continue;

        uint8_t *device_id_bytes = device_id->bytes;
        bool invalid = FALSE;

        for (uint32_t j = 0; j < id_length; ++j)
            if (id_bytes[j] != device_id_bytes[j]) {
                invalid = TRUE;
                break;
            }
        
        if (invalid)
            continue;

        ps2_port_a = device;
        break;
    }

    if (!ps2_port_a) {
        printf("Unknown\n");
        return -1;
    }

    printf("%s\n", ps2_port_a->name);

    printk("\033[33mps2:\033[37m port-a: Setting scan code set 2... ");
    __ps2_set_scancode_set(SCANCODE_SET_2);
    printf("Ok\n");

    leds = LED_RESET;
    __ps2_update_leds();

    printk("\033[33mps2:\033[37m port-a: Ready\n");
    return 0;
}

/**
 * __set_scancode_handler
*/

void __set_scancode_handler(uint32_t *table, uint32_t scancode, uint32_t handler) {
    table[scancode] = handler;
}

/**
 * __enable_ps2_a_port
*/

void __enable_ps2_a_port(void) {
    __ps2_write_byte(PS2_STATUS_REGISTER, 0xae);
}

/**
 * __disable_ps2_a_port
*/

void __disable_ps2_a_port(void) {
    __ps2_write_byte(PS2_STATUS_REGISTER, 0xad);
}

/**
 * __enable_ps2_b_port
*/

/**
 * __disable_ps2_b_port
*/

/**
 * __ps2_write_byte
*/

void __ps2_write_byte(uint8_t r, uint8_t v) {
    for (uint32_t i = 0; i < 1000; ++i) {
        if (!(__inb(PS2_STATUS_REGISTER) & 0x02)) {
            __outb(r, v);
            return;
        }

        __delay_ms(1);
    }

    errno = ETIMEDOUT;
}

/**
 * __ps2_read_byte
*/

uint8_t __ps2_read_byte(void) {
    for (uint32_t i = 0; i < 1000; ++i) {
        if (__inb(PS2_STATUS_REGISTER) & 0x01)
            return __inb(PS2_DATA_PORT_REGISTER);

        __delay_ms(1);
    }

    errno = ETIMEDOUT;
    return 0;
}

/**
 * __ps2_update_leds
*/

void __ps2_update_leds(void) {
    for (uint32_t i = 0; i < 10; ++i) {
        __ps2_write_byte(PS2_DATA_PORT_REGISTER, PS2_SET_LEDS);
        __ps2_write_byte(PS2_DATA_PORT_REGISTER, leds);
    
        errno = 0; // reset errno

        uint8_t response = __ps2_read_byte();

        if (errno)
            continue;

        if (response == PS2_RESEND)
            continue;

        if (response != PS2_ACK) {
            printk("ps2: fatal: unknown response code: %u\n expected 0xfa or 0xfe\n", response);
            errno = EIO;
        }

        return;
    }
}

/**
 * __ps2_set_leds
*/

void __ps2_set_leds(uint8_t l) {
    leds = l & LEDS_MASK;
    __ps2_update_leds();
}

/**
 * __ps2_echo
*/

void __ps2_echo(void) {
    for (uint32_t i = 0; i < 10; ++i) {
        __ps2_write_byte(PS2_DATA_PORT_REGISTER, PS2_ECHO);
    
        errno = 0; // reset errno

        uint8_t response = __ps2_read_byte();

        if (errno)
            continue;

        if (response == PS2_RESEND)
            continue;

        if (response != PS2_ECHO) {
            printk("ps2: fatal: unknown response code: %u\n expected 0xee\n", response);
            errno = EIO;
        }

        return;
    }
}

/**
 * __ps2_get_scancode_set
*/

int8_t __ps2_get_scancode_set(void) {
    for (uint32_t i = 0; i < 10; ++i) {
        __ps2_write_byte(PS2_DATA_PORT_REGISTER, PS2_SCANCODE_SET);
        __ps2_write_byte(PS2_DATA_PORT_REGISTER, PS2_GET_SCANCODE_SET);
    
        errno = 0; // reset errno

        uint8_t response = __ps2_read_byte();

        if (errno)
            continue;

        // FIXME: this might be wrong according to osdev.org/PS/2_Keyboard
        if (response == PS2_RESEND)
            continue;

        if (response != PS2_ACK) {
            printk("ps2: fatal: unknown response code: %u\n expected 0xfa or 0xfe\n", response);
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
}

/**
 * __ps2_set_scancode_set
*/

void __ps2_set_scancode_set(uint8_t scancode_set) {
    if (scancode_set >= PS2_SCANCODE_SETS_COUNT)
        return;

    ++scancode_set;

    for (uint32_t i = 0; i < 10; ++i) {
        __ps2_write_byte(PS2_DATA_PORT_REGISTER, PS2_SCANCODE_SET);
        __ps2_write_byte(PS2_DATA_PORT_REGISTER, scancode_set);
    
        errno = 0; // reset errno

        uint8_t response = __ps2_read_byte();

        if (errno)
            continue;

        if (response == PS2_RESEND)
            continue;

        if (response != PS2_ACK) {
            printk("ps2: fatal: unknown response code: %u\n expected 0xfa or 0xfe\n", response);
            errno = EIO;
        }

        return;
    }
}

/**
 * __ps2_identify
*/

PS2_ID *__ps2_identify(void) {
    static PS2_ID id;

    for (uint32_t i = 0; i < 10; ++i) {
        errno = 0;

        __ps2_disable_scanning();

        if (errno)
            continue;

        __ps2_write_byte(PS2_DATA_PORT_REGISTER, PS2_IDENTIFY);

        if (errno)
            continue;

        uint8_t response = __ps2_read_byte();

        if (errno)
            continue;

        if (response == PS2_RESEND)
            continue; // RFC: does identify send 0xfe?

        if (response != PS2_ACK) {
            printk("ps2: fatal: unknown response code: %u\n expected 0xfa or 0xfe\n", response);
            errno = EIO;
            return NULL;
        }

        bool error = FALSE;

        for (uint32_t i = 0; i < sizeof(id) / sizeof(uint8_t); ++i) {
            uint8_t id_byte = __ps2_read_byte();

            if (errno == ETIMEDOUT) {
                id.length = i;
                // RFC: fill with zeroes?
                //do id[i++] = 0x00; while (i < sizeof(id) / sizeof(uint8_t));
                break;
            }

            if (errno) {
                error = TRUE;
                printk("ps2: error: hardware failure while reading id\n");
                break;
            }

            id.bytes[i] = id_byte;
        }

        if (error) // errno && errno != ETIMEDOUT could be used alternatively
            continue;

        // reset, errno may be set
        errno = 0;
        
        __ps2_enable_scanning();

        if (errno)
            continue;

        return &id;
    }

    return NULL;
}

/**
 * __ps2_enable_scanning
*/

void __ps2_enable_scanning(void) {
    for (uint32_t i = 0; i < 10; ++i) {
        __ps2_write_byte(PS2_DATA_PORT_REGISTER, PS2_ENABLE_SCANNING);
    
        errno = 0; // reset errno

        uint8_t response = __ps2_read_byte();

        if (errno)
            continue;

        if (response == PS2_RESEND)
            continue;

        if (response != PS2_ACK) {
            printk("ps2: fatal: unknown response code: %u\n expected 0xfa or 0xfe\n", response);
            errno = EIO;
        }

        return;
    }
}

/**
 * __ps2_disable_scanning
*/

void __ps2_disable_scanning(void) {
    for (uint32_t i = 0; i < 10; ++i) {
        __ps2_write_byte(PS2_DATA_PORT_REGISTER, PS2_DISABLE_SCANNING);
    
        errno = 0; // reset errno

        uint8_t response = __ps2_read_byte();

        if (errno)
            continue;

        if (response == PS2_RESEND)
            continue;

        if (response != PS2_ACK) {
            printk("ps2: fatal: unknown response code: %u\n", response);
            printk("expected 0xfa or 0xfe\n");
            errno = EIO;
        }

        return;
    }
}

/**
 * __ps2_reset
*/

int32_t __ps2_reset(void) {
    for (uint32_t i = 0; i < 10; ++i) {
        errno = 0;

        __ps2_write_byte(PS2_DATA_PORT_REGISTER, PS2_RESET);

        if (errno)
            continue;

        uint8_t response = __ps2_read_byte();

        if (errno || response == PS2_RESEND)
            continue;

        if (response != PS2_ACK) {
            printk("ps2: hardware failure: unknown response code: %u\n", response);
            printk("expected 0xfa or 0xfe\n");
            errno = EIO;
            return -1;
        }

        uint8_t result_code = __ps2_read_byte();

        if (errno || result_code == PS2_SELF_TEST_FAIL1 || result_code == PS2_SELF_TEST_FAIL2)
            continue;

        if (result_code != PS2_SELF_TEST_OK) {
            printk("ps2: hardware failure: unknown result code: %u\n", response);
            printk("expected 0xaa, 0xfc or 0xfd\n");
            errno = EIO;
            return -2;
        }
        return 0;
    }

    printk("ps2: failed to perform reset & self test\n");
    errno = ETIMEDOUT;
    return -3;
}