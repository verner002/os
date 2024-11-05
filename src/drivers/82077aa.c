/**
 * 82077aa
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/82077aa.h"

/**
 * Static Global Variables
*/

static struct {
    byte driveSelect;
    bool operationMode; // clear = reset mode, set = normal mode
    bool irqsEnabled;
    bool irqReceived;
    DRIVE master;
    DRIVE slave;
} controller;

/**
 * __detect_drives
*/

void __detect_drives(void) {
    byte v = __read_cmos_register(0x10);

    controller.master.type = (DRIVE_TYPE)(v >> 0x04);
    controller.slave.type = (DRIVE_TYPE)(v & 0x0f);
}

/**
 * __set_datarate
*/

int __set_datarate(DRIVE drive) {
    switch (drive.type) {
        case DRIVE_TYPE_NO_DRIVE: break;
        case DRIVE_TYPE_12MB_525:
        case DRIVE_TYPE_144MB_35:
            __outb(FDC_DATARATE_SELECT_REGISTER, 0x00);
            __outb(FDC_CONFIGURATION_CONTROL_REGISTER, 0x00);
            break;
        case DRIVE_TYPE_288MB_35:
            __outb(FDC_DATARATE_SELECT_REGISTER, 0x03);
            __outb(FDC_CONFIGURATION_CONTROL_REGISTER, 0x03);
            break;
        default: return -1;
    }

    return 0;
}

/**
 * __update_dor
*/

void __update_dor(void) {
    __outb(FDC_DIGITAL_OUTPUT_REGISTER, (controller.slave.motorOn << 5) | (controller.master.motorOn << 4) | (controller.irqsEnabled << 3) | (controller.operationMode << 2) | (controller.driveSelect & 0x03));
}

/**
 * __turn_motor_on
*/

void __turn_motor_on(DRIVE drive) {
    drive.motorOn = TRUE;
    __update_dor();
}

/**
 * __turn_motor_on
*/

void __turn_motor_off(DRIVE drive) {
    drive.motorOn = FALSE;
    __update_dor();
}

/**
 * __irq_handler
*/

void __irq_handler(void) {
    controller.irqReceived = TRUE;
}

/**
 * __fdc_reset
*/

static void __fdc_reset(void) {
    controller.irqReceived = FALSE;

    controller.slave.motorOn = FALSE;
    controller.master.motorOn = FALSE;
    controller.irqsEnabled = FALSE;
    controller.operationMode = FALSE; // reset mode
    controller.driveSelect = 0x00;
    __update_dor();

    __wait_for_fdc(); // 4 useconds delay

    controller.irqsEnabled = TRUE;
    controller.operationMode = TRUE; // normal mode
    __update_dor();

    while (!controller.irqReceived); // wait for irq 6
}

/**
 * __wait_for_fdc_input_buff
*/

int __wait_for_fdc_input_buff(void) {
    while (!(__inb(FDC_MAIN_STATUS_REGISTER) & 0x80)); // wait for fdc

    if (!(__inb(FDC_MAIN_STATUS_REGISTER) & 0x40)) return -1; // fdc should expect an in opcode

    return 0;
}

/**
 * __write_byte
*/

void __write_byte(byte v) {
    for (unsigned int i = 0; i < 512; ++i) {
        if (__inb(FDC_MAIN_STATUS_REGISTER) & 0xc0 == 0x80) {
            __outb(FDC_DATA_FIFO, v);
            return;
        }
    }

    errno = ETIMEDOUT;
}

/**
 * __read_byte
*/

byte __read_byte(void) {
    for (unsigned int i = 0; i < 512; ++i) if (__inb(FDC_MAIN_STATUS_REGISTER) & 0xc0 == 0xc0) return __inb(FDC_DATA_FIFO);

    errno = ETIMEDOUT;
}

/**
 * __software_reset
*/

void __software_reset(void) {
    unsigned int i;
    bool try_again = FALSE;

    for (i = 0; i < 3; ++i) { // this is ugly, i'll change it later
        __fdc_reset();

        for (unsigned int j = 0; j < 4; ++j) { // we should send sense_interrupt 4x
            __write_byte(FDC_COMMAND_SENSE_INTERRUPT);
            __read_byte(); // read st0
            __read_byte(); // read cylinder
        }

        if (errno || __inb(FDC_MAIN_STATUS_REGISTER) & 0xc0 != 0x80) continue;

        __outb(FDC_DATA_FIFO, FDC_COMMAND_CONFIGURE);

        __write_byte(0x00);

        bool impliedSeekEnabled = TRUE;
        bool fifoDisabled = FALSE;
        bool drivePollingModeDisabled = TRUE;
        byte threshold = 8 - 1;

        __write_byte((impliedSeekEnabled << 6) | (fifoDisabled << 5) | (drivePollingModeDisabled << 4) | threshold);
        __write_byte(0x00); // precompensation = 0

        __write_byte(FDC_COMMAND_SPECIFY);

        // parameters for 1.44 MB floppy
        byte ms1 = 8; // time the controller should wait for the head assembly to move between successive cylinders
        byte ms2 = 240; // time the controller should wait between activating a head and actually performing a read/write, set to 0 (maximum in any mode)?
        byte ms3 = 30; // time the controller should wait before deactivating the head
        byte dr = 500000; // datarate

        byte srt = 16 - (ms1 * dr / 500000);
        byte hut = ms2 * dr / 8000000;
        byte hlt = ms3 * dr / 1000000;
        bool ndma = FALSE;

        __write_byte((srt << 4) | (hut & 0x0f));
        __write_byte((hlt << 1) | ndma);

        break;
    }

    if (i >= 3) errno = ETIMEDOUT; // timeout

    // fdc ready to recieve cmds
}

/**
 * __init_drives
*/

int __init_drives(void) {
    controller.master.id = 0x00;
    
    controller.slave.id = 0x01;

    __detect_drives();

    __set_datarate(controller.master);
    __set_datarate(controller.slave);

    return -1;
}

/**
 * __wait_for_fdc
*/

void __wait_for_fdc(void) {
    __outb(POST_CODE_REGISTER, 0x00);
}