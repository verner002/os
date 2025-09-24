/**
 * 82077aa
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
#include "drivers/cpu.h"
#include "drivers/cmos.h"
#include "drivers/dma/8237a.h"
#include "kstdlib/errno.h"
#include "kernel/mutex.h"
#include "kernel/task.h"

/**
 * Constants
*/

#define __FDC_OUTB_TIMEOUT 500
#define __FDC_INB_TIMEOUT 500
#define __FDC_IRQ6_TIMEOUT 500

// parameters for 1.44 MB floppy
#define DATARATE 500000
#define HEAD_ASSEMBLY 8 // time the controller should wait for the head assembly to move between successive cylinders
#define HEAD_ACTIVATION 30 // time the controller should wait between activating a head and actually performing a read/write, set to 0 (maximum in any mode)?
#define HEAD_DEACTIVATION 240 // time the controller should wait before deactivating the head

/**
 * Types Definitions
*/

typedef enum _drive_type DRIVE_TYPE;
typedef enum _fdc_command FDC_COMMAND;
typedef enum __fdc_command_extension FDC_COMMAND_EXTENSION;
typedef enum __fdc_gap3_length FDC_GAP3_LENGTH;
typedef enum __fdc_sector_dtl FDC_SECTOR_DTL;
typedef struct _drive DRIVE;

/**
 * Enumerations
*/

enum _drive_type {
    DRIVE_TYPE_NO_DRIVE = 0x00,
    DRIVE_TYPE_360KB_525 = 0x01,
    DRIVE_TYPE_12MB_525 = 0x02,
    DRIVE_TYPE_720KB_35 = 0x03,
    DRIVE_TYPE_144MB_35 = 0x04,
    DRIVE_TYPE_288MB_35 = 0x05
};

enum _fdc_command {
    FDC_COMMAND_READ_TRACK = 0x02, // generates irq 6
    FDC_COMMAND_SPECIFY = 0x03, // set drive parameters
    FDC_COMMAND_SENSE_DRIVE_STATUS = 0x04,
    FDC_COMMAND_WRITE_DATA = 0x05, // write to disk
    FDC_COMMAND_READ_DATA = 0x06, // read from disk
    FDC_COMMAND_RECALIBRATE = 0x07, // seek to cylinder 0
    FDC_COMMAND_SENSE_INTERRUPT = 0x08, // ack irq 6, get status of last cmd
    FDC_COMMAND_WRITE_DELETED_DATA = 0x09,
    FDC_COMMAND_READ_ID = 0x0a, // generates irq 6
    FDC_COMMAND_READ_DELETED_DATA = 0x0c,
    FDC_COMMAND_FORMAT_TRACK = 0x0d,
    FDC_COMMAND_DUMPREG = 0x0e,
    FDC_COMMAND_SEEK = 0x0f, // seek both heads to cylinder x
    FDC_COMMAND_VERSION = 0x10, // used during initialization
    FDC_COMMAND_SCAN_EQUAL = 0x11,
    FDC_COMMAND_PERPENDICULAR_MODE = 0x12, // used during initialization
    FDC_COMMAND_CONFIGURE = 0x13, // set controller parameters
    FDC_COMMAND_LOCK = 0x14, // protect controller parameters from reset
    FDC_COMMAND_VERIFY = 0x16,
    FDC_COMMAND_SCAN_LOW_OR_EQUAL = 0x19,
    FDC_COMMAND_SCAN_HIGH_OR_EQUAL = 0x1d
};

enum __fdc_command_extension {
    FDC_COMMAND_EXTENSION_SKIP = 0x20,
    FDC_COMMAND_EXTENSION_DENSITY = 0x40,
    FDC_COMMAND_EXTENSION_MULTITRACK = 0x80
};

enum __fdc_gap3_length {
    FDC_GAP3_LENGTH_STANDARD = 42,
    FDC_GAP3_LENGTH_5_14 = 32,
    FDC_GAP3_LENGTH_3_5 = 27
};

enum __fdc_sector_dtl {
    FDC_SECTOR_DTL_128 = 0,
    FDC_SECTOR_DTL_256 = 1,
    FDC_SECTOR_DTL_512 = 2,
    FDC_SECTOR_DTL_1024 = 3
};

enum __fdc_sectors_per_track {
    FDC_SECTORS_PER_TRACK_3_5 = 18
};

/**
 * Structures
*/

struct _drive {
    uint8_t id;
    DRIVE_TYPE type; // drive type from cmos
    bool motorOn;
};

/**
 * Declarations
*/

void __fdc_outb(uint8_t v);
uint8_t __fdc_inb(void);
int32_t __init_fdc(void);
int32_t __fdc_recalibrate(void);
int32_t __fdc_seek(uint32_t head, uint32_t cylinder);
int32_t __fdc_read_sectors(uint32_t lba, uint32_t count, uint32_t buffer);