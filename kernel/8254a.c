/**
 * @file 8254a.c
 * @author verner002
 * @date 06/05/2026
*/

#include "kernel/io.h"
#include "kernel/8254a.h"
#include "kernel/vga.h"

#define PIT_CHANNEL_0_DATA_REGISTER     0x40
#define PIT_CHANNEL_2_DATA_REGISTER     0x42
#define PIT_MODE_COMMAND_REGISTER       0x43

void pit_init(void) {
    // channel 0
    // low/high byte + rate generator
    // low byte = 0, high byte = 0
    outb(PIT_MODE_COMMAND_REGISTER, 0x34);
    outb(PIT_CHANNEL_0_DATA_REGISTER, 0x00);
    outb(PIT_CHANNEL_0_DATA_REGISTER, 0x00);
    
    // channel 1
    // we don't use this one

    // channel 2
    // low/high byte, square wave
    // low byte = 0, high byte = 0
    outb(PIT_MODE_COMMAND_REGISTER, 0xb6);
    outb(PIT_CHANNEL_2_DATA_REGISTER, 0x00);
    outb(PIT_CHANNEL_2_DATA_REGISTER, 0x00);
}

/*void pit_config_channel(int channel, int mode, uint16_t value) {
    switch (channel) {
        case PIT_CHANNEL_0:
            break;

        case PIT_CHANNEL_1:
            vga_write("pit: attempted to access channel 1\n");
            return;

        case PIT_CHANNEL_2:
            break;
    }
}*/