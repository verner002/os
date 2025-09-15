/**
 * @file ide.c
 * @author verner002
 * @date 04/09/2025
*/

#include "bool.h"
#include "macros.h"
#include "drivers/ide.h"
#include "drivers/cpu.h"
#include "drivers/ports.h"
#include "kernel/kdev.h"
#include "hal/dev.h"

#define IDE_CHANNEL_0 0x00
#define IDE_CHANNEL_1 0x01

// prog if
#define IDE_CHANNEL_0_MODE 0x01
#define IDE_CHANNEL_0_SWITCHING 0x02
#define IDE_CHANNEL_1_MODE 0x04
#define IDE_CHANNEL_1_SWITCHING 0x08
#define IDE_BUS_MASTERING 0x80
#define IDE_CHANNEL_0_IO_BASE 0x01f0
#define IDE_CHANNEL_0_CTRL_BASE 0x03f6
#define IDE_CHANNEL_1_IO_BASE 0x0170
#define IDE_CHANNEL_1_CTRL_BASE 0x0376

#define IDE_DATA_REG        0x00
#define IDE_ERROR_REG       0x01
#define IDE_FEATURES_REG    0x01
#define IDE_SECCOUNT0_REG   0x02
#define IDE_LBA0_REG        0x03
#define IDE_LBA1_REG        0x04
#define IDE_LBA2_REG        0x05
#define IDE_HDDEVSEL_REG    0x06
#define IDE_COMMAND_REG     0x07
#define IDE_STATUS_REG      0x07
#define IDE_SECCOUNT1_REG   0x08
#define IDE_LBA3_REG        0x09
#define IDE_LBA4_REG        0x0a
#define IDE_LBA5_REG        0x0b
#define IDE_CONTROL_REG     0x0c
#define IDE_ALTSTATUS_REG   0x0c
#define IDE_DEVADDRESS_REG  0x0d

#define IDE_IDENTIFY_CMD    0xec

#define IDE_STATUS_ERR      0x01 // error
#define IDE_STATUS_IDX      0x02 // index
#define IDE_STATUS_CORR     0x04 // corrected data
#define IDE_STATUS_DRQ      0x08 // data request ready
#define IDE_STATUS_DSC      0x10 // drive seek complete
#define IDE_STATUS_DF       0x20 // drive write fault
#define IDE_STATUS_DRDY     0x40 // drive ready
#define IDE_STATUS_BSY      0x80 // busy

#define IDE_IDENTIFY_DEVICE_TYPE    0
#define IDE_IDENTIFY_CYLINDERS      2
#define IDE_IDENTIFY_HEADS          6
#define IDE_IDENTIFY_SECTORS        12
#define IDE_IDENTIFY_SERIAL         20
#define IDE_IDENTIFY_FIRMWARE       46
#define IDE_IDENTIFY_MODEL          54
#define IDE_IDENTIFY_CAPABILITIES   98
#define IDE_IDENTIFY_FIELD_VALID    106
#define IDE_IDENTIFY_MAX_LBA        120
#define IDE_IDENTIFY_COMMAND_SETS   164
#define IDE_IDENTIFY_MAX_LBA_EXT    200

#define IDE_CMD_READ_PIO        0x20
#define IDE_CMD_READ_PIO_EXT    0x24

#define IDE_LBA48_ADDRESSING (1 << 26)

struct __ide_channel_regs {
    uint16_t r_io_base;
    uint16_t r_ctrl_base;
    uint16_t r_busm_base;
    bool r_ints_dis;
} channels[2];

uint32_t drvs_cnt = 0;

struct __ide_drive {
    uint8_t d_type;
    uint8_t d_channel;
    uint8_t d_drive;
    uint16_t d_signature;
    uint16_t d_capabilities;
    uint16_t d_command_sets;
    uint32_t d_last_sector;
    char d_model[41];
    char d_serial[21];
    char d_firmware[9];
    uint8_t d_mode; // 0 = chs, 1 = lba28, 2 = lba48
    bool d_dma; // false = not supported, true = supported 
} drives[4];

static uint8_t __ide_buffer[2048];
static uint8_t __atapi_buffer[12] = { 0xa8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static volatile bool __irq_invoked;

void __dev_type_release(struct __dev *dev) {
    struct __kobj *kobj = dev->d_kobj;

    if (kobj) {
        //dev->d_kobj->k_type->k_release(dev->d_kobj);
    } else {
        __kdev_t kdev = dev->d_kdev;
        printk("ide: warning: device %u:%u is missing kobj reference\n", MAJOR(kdev), MINOR(kdev));
    }

    kfree(dev->d_name);
}

void __ide_write(uint8_t channel, uint8_t reg, uint8_t data) {
    if (reg > 0x07 && reg < 0x0C)
        __ide_write(channel, IDE_CONTROL_REG, 0x80 | channels[channel].r_ints_dis);
    
    if (reg < 0x08)
        __outb(channels[channel].r_io_base  + reg, data);
    else if (reg < 0x0c)
        __outb(channels[channel].r_io_base  + reg - 0x06, data);
    else if (reg < 0x0e)
        __outb(channels[channel].r_ctrl_base  + reg - 0x0a, data);
    else if (reg < 0x16)
        __outb(channels[channel].r_busm_base + reg - 0x0e, data);

    if (reg > 0x07 && reg < 0x0c)
        __ide_write(channel, IDE_CONTROL_REG, channels[channel].r_ints_dis);
}

uint8_t __ide_read(uint8_t channel, uint8_t reg) {
    uint8_t data = 0;

    if (reg > 0x07 && reg < 0x0C)
        __ide_write(channel, IDE_CONTROL_REG, 0x80 | channels[channel].r_ints_dis);

    else if (reg < 0x08)
        data = __inb(channels[channel].r_io_base + reg);
    else if (reg < 0x0c)
        data = __inb(channels[channel].r_io_base + reg - 0x06);
    else if (reg < 0x0e)
        data = __inb(channels[channel].r_ctrl_base  + reg - 0x0a);
    else if (reg < 0x16)
        data = __inb(channels[channel].r_busm_base + reg - 0x0e);

    if (reg > 0x07 && reg < 0x0c)
        __ide_write(channel, IDE_CONTROL_REG, channels[channel].r_ints_dis);

    return data;
}

/**
 * __ide_strcpy
*/

void __ide_strncpy(char *dest, char const *src, uint32_t len) {
    for (uint32_t i = 0; i < 40; i += 2) {
        dest[i] = src[i + 1];
        dest[i + 1] = src[i];
    }

    // ignore trailing spaces
    uint32_t i = len;

    for (; i > 0; --i)
        if (dest[i - 1] != ' ')
            break;

    dest[i] = '\0';
}

/** 
 * __ide_identify_drives
*/

void __ide_identify_drives(void) {
    for (uint8_t ch = 0; ch < 2; ++ch) {
        for (uint8_t drv = 0; drv < 2; ++drv) {
            drives[drvs_cnt].d_type = 0;

            __ide_write(ch, IDE_HDDEVSEL_REG, 0xa0 | (drv << 4));
            __delay_ms(1);

            __ide_write(ch, IDE_LBA0_REG, 0x00);
            __ide_write(ch, IDE_LBA1_REG, 0x00);
            __ide_write(ch, IDE_LBA2_REG, 0x00);

            __ide_write(ch, IDE_COMMAND_REG, IDE_IDENTIFY_CMD);
            __delay_ms(1);

            if (!__ide_read(ch, IDE_STATUS_REG))
                continue; // drive does not exist

            // poll until not busy
            while (__ide_read(ch, IDE_STATUS_REG) & IDE_STATUS_BSY);

            if (__ide_read(ch, IDE_LBA1_REG) || __ide_read(ch, IDE_LBA2_REG))
                continue; // not ata (atapi?)

            bool error = FALSE;

            // TODO: implement timeout
            for (;;) {
                uint8_t status = __ide_read(ch, IDE_STATUS_REG);

                // pata drives should set drq
                // sata, patapi and satapi should set err
                if (status & IDE_STATUS_ERR) {
                    error = TRUE;
                    break;
                } else if (status & IDE_STATUS_DRQ)
                    break;
            }

            uint8_t type;

            if (error) {
                uint8_t cl = __ide_read(ch, IDE_LBA1_REG);
                uint8_t ch = __ide_read(ch, IDE_LBA2_REG);

                if (cl == 0x00 && ch == 0x00)
                    type = 1;
                else if (cl == 0x14 && ch == 0xeb)
                    type = 2; // patapi
                else if (cl == 0x3c && ch == 0xc3)
                    type = 3; // sata
                else if (cl == 0x69 && ch == 0x96)
                    type = 4; // satapi
                else
                    continue; // unknown
            } else
                type = 1; // pata

            drives[drvs_cnt].d_type = type;
            drives[drvs_cnt].d_channel = ch;
            drives[drvs_cnt].d_drive = drv;

            for (uint32_t i = 0; i < 256; ++i)
                ((uint16_t *)__ide_buffer)[i] = __inw(channels[ch].r_io_base + IDE_DATA_REG);

            drives[drvs_cnt].d_signature = *((uint16_t *)(__ide_buffer + IDE_IDENTIFY_DEVICE_TYPE));
            drives[drvs_cnt].d_capabilities = *((uint16_t *)(__ide_buffer + IDE_IDENTIFY_CAPABILITIES));

            uint16_t command_sets = *((uint16_t *)(__ide_buffer + IDE_IDENTIFY_COMMAND_SETS));
            drives[drvs_cnt].d_command_sets = command_sets;

            // FIXME: this is not handled properly, i think we should add
            //  an empty_drive identificator or use another format
            uint32_t sectors;

            if (command_sets & IDE_LBA48_ADDRESSING) {
                if (*((uint32_t *)(__ide_buffer + IDE_IDENTIFY_MAX_LBA_EXT + sizeof(uint32_t))))
                    sectors = 0xffffffff; // we are limited to 32 bits
                else
                    sectors = *((uint32_t *)(__ide_buffer + IDE_IDENTIFY_MAX_LBA_EXT));
            } else
                sectors = *((uint32_t *)(__ide_buffer + IDE_IDENTIFY_MAX_LBA));

            // drive could report 0 sectors (empty qemu image)
            drives[drvs_cnt].d_last_sector = sectors ? sectors - 1 : 0;

            __ide_strncpy(drives[drvs_cnt].d_model, __ide_buffer + IDE_IDENTIFY_MODEL, 40);
            __ide_strncpy(drives[drvs_cnt].d_serial, __ide_buffer + IDE_IDENTIFY_SERIAL, 20);
            __ide_strncpy(drives[drvs_cnt].d_firmware, __ide_buffer + IDE_IDENTIFY_FIRMWARE, 8);

            uint8_t mode;

            if (drives[drvs_cnt].d_capabilities & 0x0200) {
                if (drives[drvs_cnt].d_command_sets & 0x0400)
                    mode = 2;
                else
                    mode = 1;
            } else
                mode = 0;

            drives[drvs_cnt].d_mode = mode;
            drives[drvs_cnt].d_dma = drives[drvs_cnt].d_capabilities & 0x0100;

            printf("Found %s %s %s\n", (char *[]){ "primary", "secondary" }[ch], (char *[]){ "master", "slave" }[drv], (char *[]){ "Unknown", "PATA", "SATA", "PATAPI", "SATAPI" }[type]);
            printf(" - Model      : %s\n", drives[drvs_cnt].d_model);
            printf(" - Signature  : %04x\n", drives[drvs_cnt].d_signature);
            printf(" - Last sector: %u\n", drives[drvs_cnt].d_last_sector);
            printf(" - Serial     : %s\n", drives[drvs_cnt].d_serial);
            printf(" - Firmware   : %s\n", drives[drvs_cnt].d_firmware);
            printf(" - Mode       : %s\n", (char *[]){ "CHS (Unsupported)", "LBA28", "LBA48" }[mode]);
            printf(" - DMA        : %s\n", drives[drvs_cnt].d_dma ? "supported" : "unsupported");

            /*struct __dev *dev = (struct __dev *)kmalloc(sizeof(struct __dev));

            if (!dev) {
                printk("ide: warning: failed to allocate memory for device struct\n");
                continue; // we can try to detect other devices
            }

            dev->d_bus = b;
            dev->d_name = drives[drvs_cnt].d_model;
            dev->d_id = __dev_id;
            atomic_inc(__dev_id);*/
            
            ++drvs_cnt;
        }
    }
}

/**
 * __ide_poll
*/

int32_t __ide_poll(uint8_t channel, bool check_state) {
    for (uint32_t i = 0; i < 4; ++i)
        __ide_read(channel, IDE_ALTSTATUS_REG);

    while (__ide_read(channel, IDE_STATUS_REG) & IDE_STATUS_BSY);

    if (!check_state)
        return 0;

    uint8_t status = __ide_read(channel, IDE_STATUS_REG);

    if (status & IDE_STATUS_ERR)
        return -1; // error (read error register)
    else if (status & IDE_STATUS_DF)
        return -2; // device faault
    else if (!(status & IDE_STATUS_DRQ))
        return -3; // data request not ready

    return 0;
}

/**
 * __ide_read_block
*/

int32_t __ide_read_blocks(uint8_t drive, uint32_t lba, uint8_t count, uint8_t *buffer) {
    if (!count)
        return -1; // count must be positive

    struct __ide_drive *drv = &drives[drive];

    if (lba + count - 1 > drv->d_last_sector)
        return -2; // out of disk space

    uint8_t mode = drv->d_mode;

    if (!mode)
        return -3; // chs mode not supported

    uint16_t c = 0;
    uint8_t h;
    uint8_t s = 0;
    uint8_t lbas[6];

    switch (mode) {
        case 2: // lba48
            lbas[0] = (lba & 0x000000ff) >> 0;
            lbas[1] = (lba & 0x0000ff00) >> 8;
            lbas[2] = (lba & 0x00ff0000) >> 16;
            lbas[3] = (lba & 0xff000000) >> 24;
            lbas[4] = 0;
            lbas[5] = 0;
            h = 0;
            break;

        case 1: // lba28
            lbas[0] = (lba & 0x000000ff) >> 0;
            lbas[1] = (lba & 0x0000ff00) >> 8;
            lbas[2] = (lba & 0x00ff0000) >> 16;
            lbas[3] = 0;
            lbas[4] = 0;
            lbas[5] = 0;
            h = (lba & 0x0f000000) >> 24;
            break;
    }

    uint8_t channel = drv->d_channel;
    
    while (__ide_read(channel, IDE_STATUS_REG) & IDE_STATUS_BSY);

    __ide_write(channel, IDE_HDDEVSEL_REG, 0xe0 | ((drv->d_drive & 0x01) << 4) | h);

    if (mode == 2) {
        __ide_write(channel, IDE_SECCOUNT1_REG, 0);
        __ide_write(channel, IDE_LBA3_REG, lbas[3]);
        __ide_write(channel, IDE_LBA4_REG, lbas[4]);
        __ide_write(channel, IDE_LBA5_REG, lbas[5]);
    }

    __ide_write(channel, IDE_SECCOUNT0_REG, s);
    __ide_write(channel, IDE_LBA0_REG, lbas[0]);
    __ide_write(channel, IDE_LBA1_REG, lbas[1]);
    __ide_write(channel, IDE_LBA2_REG, lbas[2]);

    __ide_write(channel, IDE_COMMAND_REG, mode == 2 ? IDE_CMD_READ_PIO_EXT : IDE_CMD_READ_PIO);

    uint16_t *offset = (uint16_t *)buffer;
    uint16_t port = channels[channel].r_io_base;

    for (uint32_t i = 0; i < count; ++i) {
        if (__ide_poll(channel, TRUE))
            return -4;

        asm (
            "rep insw"
            :
            : "c" (256), "d" (port), "D" (offset)
            :
        );

        offset += 256;
    }

    return 0;
}

/**
 * __init_ide
*/

int32_t __init_ide(struct __bus *b, struct __pci_header *h) {
    printf("        Initializing...\n");

    struct __pci_h_device *d = (struct __pci_h_device *)h;

    uint8_t progif = d->h.h_prog_if;

    uint8_t ch0_mode = progif & IDE_CHANNEL_0_MODE;
    uint8_t ch0_switch = progif & IDE_CHANNEL_0_SWITCHING;
    uint8_t ch1_mode = progif & IDE_CHANNEL_1_MODE;
    uint8_t ch1_switch = progif & IDE_CHANNEL_1_SWITCHING;

    printf("        Channel 0 mode          : %s\n", ch0_mode ? "native" : "compatibility");
    printf("        Channel 0 mode switching: %s\n", ch0_switch ? "enabled" : "disabled");

    printf("        Channel 1 mode          : %s\n", ch1_mode ? "native" : "compatibility");
    printf("        Channel 1 mode switching: %s\n", ch1_switch ? "enabled" : "disabled");

    printf("        Bus mastering           : %s\n", progif & IDE_BUS_MASTERING ? "supported" : "unsupported");

    uint16_t io_base_0 = IDE_CHANNEL_0_IO_BASE;
    uint16_t ctrl_base_0 = IDE_CHANNEL_0_CTRL_BASE;

    if (ch0_mode) {
        io_base_0 += __pci_fix_bar(d->d_bar0);
        ctrl_base_0 += __pci_fix_bar(d->d_bar1);
    }
        
    uint16_t io_base_1 = IDE_CHANNEL_1_IO_BASE;
    uint16_t ctrl_base_1 = IDE_CHANNEL_1_CTRL_BASE;

    if (ch1_mode) {
        io_base_1 += __pci_fix_bar(d->d_bar2);
        ctrl_base_1 += __pci_fix_bar(d->d_bar3);
    }

    uint16_t busm_base_0 = 0;
    uint16_t busm_base_1 = 0;

    if (progif & IDE_BUS_MASTERING) {
        busm_base_0 += __pci_fix_bar(d->d_bar4);
        busm_base_1 += busm_base_0 + 8;
    }

    channels[IDE_CHANNEL_0].r_io_base = io_base_0;
    channels[IDE_CHANNEL_0].r_ctrl_base = ctrl_base_0;
    channels[IDE_CHANNEL_0].r_busm_base = busm_base_0;

    channels[IDE_CHANNEL_1].r_io_base = io_base_1;
    channels[IDE_CHANNEL_1].r_ctrl_base = ctrl_base_1;
    channels[IDE_CHANNEL_1].r_busm_base = busm_base_1;

    // disable interrupts
    __ide_write(IDE_CHANNEL_0, IDE_CONTROL_REG, 0x02);
    __ide_write(IDE_CHANNEL_1, IDE_CONTROL_REG, 0x02);

    // identify drives
    __ide_identify_drives();
    
    return 0;
}