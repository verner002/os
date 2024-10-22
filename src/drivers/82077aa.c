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
    __outb(FDC_DIGITAL_OUTPUT_REGISTER, (0x20 * controller.slave.motorOn) | (0x10 * controller.master.motorOn) | (0x08 * controller.irqsEnabled) | (0x04 * controller.operationMode) | (controller.driveSelect & 0x03));
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
 * __software_reset
*/

void __software_reset(void) {
    controller.irqReceived = FALSE;

    controller.slave.motorOn = FALSE;
    controller.master.motorOn = FALSE;
    controller.irqsEnabled = FALSE;
    controller.operationMode = FALSE; // reset mode
    controller.driveSelect = 0x00;
    __update_dor();

    // delay

    controller.irqsEnabled = TRUE;
    controller.operationMode = TRUE; // normal mode
    __update_dor();

    while (!controller.irqReceived);


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