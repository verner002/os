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

int __init_ps2(void) {
    __disable_ps2_a_port();

    __wait_for_ps2_input_buff();
    __outb(PS2_COMMAND_REGISTER, 0xa7); // disable port b, we won't use it

    __inb(PS2_DATA_PORT_REGISTER); // flush out buffer

    __wait_for_ps2_input_buff();
    __outb(PS2_COMMAND_REGISTER, 0x20); // read config byte

    __wait_for_ps2_ouput_buff();
    byte config_byte = __inb(PS2_DATA_PORT_REGISTER) & 0x24; // disable translantion for port a, disable irqs, enable signal for port a
    
    __wait_for_ps2_input_buff();
    __outb(PS2_COMMAND_REGISTER, 0x60); // write config byte
    
    __wait_for_ps2_input_buff();
    __outb(PS2_DATA_PORT_REGISTER, config_byte);

    __wait_for_ps2_input_buff();
    __outb(PS2_COMMAND_REGISTER, 0xaa); // test ps/2 controller

    __wait_for_ps2_ouput_buff();

    if (__inb(PS2_DATA_PORT_REGISTER) != 0x55) return -1;

    __wait_for_ps2_input_buff();
    __outb(PS2_COMMAND_REGISTER, 0xab); // test port a

    __wait_for_ps2_ouput_buff();

    if (__inb(PS2_DATA_PORT_REGISTER)) return -1;

    __wait_for_ps2_input_buff();
    __outb(PS2_COMMAND_REGISTER, 0x20); // read config byte

    __wait_for_ps2_ouput_buff();
    config_byte = __inb(PS2_DATA_PORT_REGISTER) | 0x01; // enable irq for port a

    __wait_for_ps2_input_buff();
    __outb(PS2_COMMAND_REGISTER, 0x60); // write config byte
    
    __wait_for_ps2_input_buff();
    __outb(PS2_DATA_PORT_REGISTER, config_byte);

    __enable_ps2_a_port();
}

/**
 * __enable_ps2_a_port
*/

void __enable_ps2_a_port(void) {
    __wait_for_ps2_input_buff();
    __outb(PS2_STATUS_REGISTER, 0xae);
}

/**
 * __disable_ps2_a_port
*/

void __disable_ps2_a_port(void) {
    __wait_for_ps2_input_buff();
    __outb(PS2_STATUS_REGISTER, 0xad);
}

/**
 * __enable_ps2_b_port
*/

/**
 * __disable_ps2_b_port
*/

/**
 * __wait_for_ps2_input_buff
*/

void __wait_for_ps2_input_buff(void) {
    while (__inb(PS2_STATUS_REGISTER) & 0x02); // TODO: add timeout
}

/**
 * __wait_for_ps2_output_buff
*/

void __wait_for_ps2_ouput_buff(void) {
    while (!(__inb(PS2_STATUS_REGISTER) & 0x01)); // TODO: add timeout
}