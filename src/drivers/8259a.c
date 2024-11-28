/**
 * 8259a
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/8259a.h"

/**
 * __init_pics
*/

void __init_pics(byte master_vec_offset, byte slave_vec_offset) {
    byte
        master_mask = __inb(PIC_MASTER_DATA_REGISTER),
        slave_mask = __inb(PIC_SLAVE_DATA_REGISTER);

    __outb(PIC_MASTER_COMMAND_REGISTER, 0x11); // initialize master pic, icw4
    __outb(PIC_SLAVE_COMMAND_REGISTER, 0x11); // initialize slave pic, icw4
    __outb(PIC_MASTER_DATA_REGISTER, master_vec_offset); // set master's vector offset icw
    __outb(PIC_SLAVE_DATA_REGISTER, slave_vec_offset); // set slave's vector offset icw
    __outb(PIC_MASTER_DATA_REGISTER, 0x04); // set master's cascade icw
    __outb(PIC_SLAVE_DATA_REGISTER, 0x02); // set slave's cascade icw
    __outb(PIC_MASTER_DATA_REGISTER, 0x01); // set master's 80x86 icw
    __outb(PIC_SLAVE_DATA_REGISTER, 0x01); // set slave's 80x86 icw

    __outb(PIC_SLAVE_DATA_REGISTER, slave_mask); // restore slaves's mask
    __outb(PIC_MASTER_DATA_REGISTER, master_mask); // restore master's mask
}

/**
 * __send_eoi
*/

void __send_eoi(byte irq_number) {
    //if (irq_number > 0x0f) return;

    if (irq_number >= 0x08) __outb(PIC_SLAVE_COMMAND_REGISTER, 0x20); // end of interrupt command

    __outb(PIC_MASTER_COMMAND_REGISTER, 0x20); // end of interrupt command
}

/**
 * __enable_irqs
*/

void __enable_irqs(void) {
    __outb(PIC_MASTER_DATA_REGISTER, 0x00);
    __outb(PIC_SLAVE_DATA_REGISTER, 0x00);
}

/**
 * __disable_irqs
*/

void __disable_irqs(void) {
    __outb(PIC_MASTER_DATA_REGISTER, 0xff);
    __outb(PIC_SLAVE_DATA_REGISTER, 0xff);
}

/**
 * __enable_irq
*/

void __enable_irq(byte irq_number) {
    word mask = ~(0x01 << irq_number) & (__inb(PIC_SLAVE_DATA_REGISTER) << 0x08 | __inb(PIC_MASTER_DATA_REGISTER));

    __outb(PIC_MASTER_DATA_REGISTER, (byte)mask);
    __outb(PIC_SLAVE_DATA_REGISTER, (byte)(mask >> 0x08));
}

/**
 * __disable_irq
*/

void __disable_irq(byte irq_number) {
    word mask = 0x01 << irq_number | (__inb(PIC_SLAVE_DATA_REGISTER) << 0x08 | __inb(PIC_MASTER_DATA_REGISTER));

    __outb(PIC_MASTER_DATA_REGISTER, (byte)mask);
    __outb(PIC_SLAVE_DATA_REGISTER, (byte)(mask >> 0x08));
}