/**
 * CMOS
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/cmos.h"

/**
 * __write_cmos_register
*/

void __write_cmos_register(byte r, byte v) {
    __disable_interrupts();
    __outb(CMOS_ADDRESS_REGISTER, r);
    __wait_for_cmos();
    __outb(CMOS_SELECTED_REGISTER, v);
    __enable_interrupts();
}

/**
 * __read_cmos_register
*/

byte __read_cmos_register(byte r) {
    __disable_interrupts();
    __outb(CMOS_ADDRESS_REGISTER, r);
    __wait_for_cmos();
    byte v = __inb(CMOS_SELECTED_REGISTER);
    __enable_interrupts();

    return v;
}

/**
 * __wait_for_cmos
*/

void __wait_for_cmos(void) {
    __outb(POST_CODE_REGISTER, 0x00);
}