/**
 * @file cmos.c
 * @author verner002
 * @date 23/04/2026
*/

#include <kernel/cmos.h>
#include <kernel/cpu.h>
#include <kernel/io.h>

#define CMOS_ADDRESS_REG        0x0070
#define CMOS_SELECT_REG         0x0071

#define CMOS_STATUS_A_REG       0x0a

static volatile int nmi_flag = 1;

/**
 * @name cmos_write
 * @arg uint8_t reg CMOS register to write to
 * @arg uint8_t value value to be written to the register
*/

void cmos_write(uint8_t reg, uint8_t value) {
    unsigned long flags;

    save_flags(flags);
    cli();

    outb(CMOS_ADDRESS_REG, ((nmi_flag ? 0x00 : 0x80)) | reg);
    io_wait();

    outb(CMOS_SELECT_REG, value);
    io_wait();
    
    restore_flags(flags);
}

/**
 * @name cmos_read
 * @arg uint8_t CMOS register to be read from
 * @return uint8_t value read from the register
*/

uint8_t cmos_read(uint8_t reg) {
    unsigned long flags;

    save_flags(flags);
    cli();

    outb(CMOS_ADDRESS_REG, ((nmi_flag ? 0x00 : 0x80)) | reg);
    io_wait();

    uint8_t value = inb(CMOS_SELECT_REG);

    restore_flags(flags);
    return value;
}

/**
 * @name cmos_update_in_progress
 * @return bool true if update is in progress, false otherwise
*/

int cmos_update_in_progress(void) {
    return (cmos_read(CMOS_STATUS_A_REG) & 0x80);
}