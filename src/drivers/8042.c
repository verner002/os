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
    return 0;
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
    for (uint32_t i = 0; i < 512; ++i) {
        if (!(__inb(PS2_STATUS_REGISTER) & 0x02)) {
            __outb(r, v);
            return;
        }
    }

    errno = ETIMEDOUT;
}

/**
 * __ps2_read_byte
*/

uint8_t __ps2_read_byte(void) {
    for (uint32_t i = 0; i < 512; ++i) if (__inb(PS2_STATUS_REGISTER) & 0x01) return __inb(PS2_DATA_PORT_REGISTER);

    errno = ETIMEDOUT;
    return 0;
}