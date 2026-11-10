/**
 * @file ide.c
 * @author verner002
 * @date 04/09/2025
*/

#include <stddef.h>

#include <kernel/ide.h>
#include <kernel/types.h>
#include <kernel/system.h>
#include <kernel/idt.h>
#include <kernel/irqs.h>
#include <kernel/io.h>
#include <kernel/cpu.h>
#include <kernel/pci.h>
#include <kernel/process.h>
#include <kernel/printk.h>

#define IDE_MAX_CHANNELS                2
#define IDE_MAX_DRIVES_PER_CHANNEL      2

#define IDE_CHANNEL_0 0x00
#define IDE_CHANNEL_1 0x01

// prog if
#define IDE_CHANNEL_0_MODE          0x01
#define IDE_CHANNEL_0_SWITCHING     0x02
#define IDE_CHANNEL_1_MODE          0x04
#define IDE_CHANNEL_1_SWITCHING     0x08
#define IDE_BUS_MASTERING           0x80
#define IDE_CHANNEL_0_IO_BASE       0x01f0
#define IDE_CHANNEL_0_CTRL_BASE     0x03f6
#define IDE_CHANNEL_1_IO_BASE       0x0170
#define IDE_CHANNEL_1_CTRL_BASE     0x0376

// ide regs
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

// cmds
#define IDE_IDENTIFY_CMD        0xec
#define IDE_READ_PIO_CMD        0x20
#define IDE_READ_PIO_EXT_CMD    0x24
#define IDE_READ_DMA_CMD        0xc8
#define IDE_READ_DMA_EXT_CMD    0x25

// flags
#define IDE_STATUS_ERR      0x01 // error
#define IDE_STATUS_IDX      0x02 // index
#define IDE_STATUS_CORR     0x04 // corrected data
#define IDE_STATUS_DRQ      0x08 // data request ready
#define IDE_STATUS_DSC      0x10 // drive seek complete
#define IDE_STATUS_DF       0x20 // drive write fault
#define IDE_STATUS_DRDY     0x40 // drive ready
#define IDE_STATUS_BSY      0x80 // busy

// fields
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

#define IDE_LBA48_ADDRESSING (1 << 26)

#define IDE_DMA_SUPPORTED 0x0100
#define IDE_PIO_SUPPORTED 0x0200

#define IDE_MODE_PIO_48 0x0400

#define IDE_IRQS_DISABLED 0x02

#define PRD_EOT 0x8000

// btw "prd" means "fart" in czech
// physical region descriptor
struct prd_entry {
    uint32_t base; // memory region base address
    uint16_t bytes; // number of bytes to transfer
    uint16_t flags; // mainly reserved, msb used as eot marker
} prdt; // single prd in prdt

struct ide_channel_regs {
    uint16_t io_base;
    uint16_t ctrl_base;
    uint16_t busm_base;
    int flags;
} channels[2];

uint32_t drvs_cnt = 0;

struct ide_drive {
    uint8_t type;
    uint8_t channel;
    uint8_t drive;
    uint16_t signature;
    uint16_t capabilities;
    uint16_t command_sets;
    uint32_t last_sector;
    char model[41];
    char serial[21];
    char firmware[9];
    uint8_t mode; // 0 = chs, 1 = lba28, 2 = lba48
    int dma; // =0 = not supported, !=0 = supported 
} drives[4];

/**
 * TODO: sort requests (in order = r1.lba <= r2.lba)
 *  in the case of hdd
*/

#define NREQ            1 //32

#define NONE_RID        -1
#define MIN_RID         0
#define MAX_RID         (NREQ - 1)

typedef int rid_t;

#define IDE_FREE        -1
#define IDE_READ        0
#define IDE_WRITE       1

static rid_t head = NONE_RID, tail = NONE_RID;

static struct semaphore reqs_sem = {
    .count = NREQ,
    .wait_list = NULL
};

static struct ide_request {
    int8_t cmd; // command (FREE=-1, READ=0, WRITE=1)
    uint8_t drv; // drive number
    uint32_t lba; // starting block
    uint32_t count; // number of blocks
    char *buffer; // pointer to buffer
    struct semaphore sem; // buffer semaphore
    rid_t previous; // previous request index
    rid_t next; // next request index
} requests[NREQ];

static uint8_t ide_buffer[2048];
static uint8_t atapi_buffer[12] = { 0xa8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static volatile int irq_invoked;

/**
 * ide_write
*/

void ide_write(uint8_t channel, uint8_t reg, uint8_t data) {
    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, IDE_CONTROL_REG, 0x80 | (channels[channel].flags & IDE_IRQS_DISABLED));
    
    if (reg < 0x08)
        outb(channels[channel].io_base  + reg, data);
    else if (reg < 0x0c)
        outb(channels[channel].io_base  + reg - 0x06, data);
    else if (reg < 0x0e)
        outb(channels[channel].ctrl_base  + reg - 0x0a, data);
    else if (reg < 0x16)
        outb(channels[channel].busm_base + reg - 0x0e, data);

    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, IDE_CONTROL_REG, channels[channel].flags & IDE_IRQS_DISABLED);
}

/**
 * ide_read
*/

uint8_t ide_read(uint8_t channel, uint8_t reg) {
    uint8_t data = 0;

    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, IDE_CONTROL_REG, 0x80 | (channels[channel].flags & IDE_IRQS_DISABLED));

    else if (reg < 0x08)
        data = inb(channels[channel].io_base + reg);
    else if (reg < 0x0c)
        data = inb(channels[channel].io_base + reg - 0x06);
    else if (reg < 0x0e)
        data = inb(channels[channel].ctrl_base  + reg - 0x0a);
    else if (reg < 0x16)
        data = inb(channels[channel].busm_base + reg - 0x0e);

    if (reg > 0x07 && reg < 0x0c)
        ide_write(channel, IDE_CONTROL_REG, channels[channel].flags & IDE_IRQS_DISABLED);

    return data;
}

/**
 * ide_strncpy
*/

void ide_strncpy(char *dest, char const *src, int len) {
    for (int i = 0; i < 40; i += 2) {
        dest[i] = src[i + 1];
        dest[i + 1] = src[i];
    }

    // ignore trailing spaces
    int i = len;

    for (; i > 0; --i)
        if (dest[i - 1] != ' ')
            break;

    dest[i] = '\0';
}

/** 
 * ide_identify_drives
*/

void ide_identify_drives(void) {
    for (uint8_t ch = 0; ch < IDE_MAX_CHANNELS; ++ch) {
        for (uint8_t drv = 0; drv < IDE_MAX_DRIVES_PER_CHANNEL; ++drv) {
            drives[drvs_cnt].type = 0;

            ide_write(ch, IDE_HDDEVSEL_REG, 0xa0 | (drv << 4));
            //__delay_ms(1);

            ide_write(ch, IDE_LBA0_REG, 0x00);
            ide_write(ch, IDE_LBA1_REG, 0x00);
            ide_write(ch, IDE_LBA2_REG, 0x00);

            ide_write(ch, IDE_COMMAND_REG, IDE_IDENTIFY_CMD);
            //__delay_ms(1);

            if (!ide_read(ch, IDE_STATUS_REG))
                continue; // drive does not exist

            // poll until not busy
            int retries = 3;
            int timeout = 0;

            while (ide_read(ch, IDE_STATUS_REG) & IDE_STATUS_BSY) {
                if (--retries) {
                    timeout = 1;
                    break;
                }
            }

            if (timeout) {
                printk("drive (%u, %u) busy (timed out)\n", ch, drv);
                continue;
            }

            if (ide_read(ch, IDE_LBA1_REG) || ide_read(ch, IDE_LBA2_REG))
                continue; // not ata (atapi?)

            retries = 3;
            //timeout = false;
            int error = 0;

            for (;;) {
                uint8_t status = ide_read(ch, IDE_STATUS_REG);

                // pata drives should set drq
                // sata, patapi and satapi should set err
                if (status & IDE_STATUS_ERR) {
                    error = 1;
                    break;
                } else if (status & IDE_STATUS_DRQ)
                    break;
                else if (--retries) {
                    timeout = 1;
                    break;
                }
            }

            if (timeout) {
                printk("drive (%u, %u) not ready (timed out)\n", ch, drv);
                continue;
            }

            uint8_t type;

            if (error) {
                uint8_t cl = ide_read(ch, IDE_LBA1_REG);
                uint8_t ch = ide_read(ch, IDE_LBA2_REG);

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

            drives[drvs_cnt].type = type;
            drives[drvs_cnt].channel = ch;
            drives[drvs_cnt].drive = drv;

            for (uint32_t i = 0; i < 256; ++i)
                ((uint16_t *)ide_buffer)[i] = inw(channels[ch].io_base + IDE_DATA_REG);

            drives[drvs_cnt].signature = *((uint16_t *)(ide_buffer + IDE_IDENTIFY_DEVICE_TYPE));
            drives[drvs_cnt].capabilities = *((uint16_t *)(ide_buffer + IDE_IDENTIFY_CAPABILITIES));

            uint16_t command_sets = *((uint16_t *)(ide_buffer + IDE_IDENTIFY_COMMAND_SETS));
            drives[drvs_cnt].command_sets = command_sets;

            // FIXME: this is not handled properly, i think we should add
            //  an empty_drive identificator or use another format
            uint32_t sectors;

            if (command_sets & IDE_LBA48_ADDRESSING) {
                if (*((uint32_t *)(ide_buffer + IDE_IDENTIFY_MAX_LBA_EXT + sizeof(uint32_t))))
                    sectors = 0xffffffff; // we are limited to 32 bits
                else
                    sectors = *((uint32_t *)(ide_buffer + IDE_IDENTIFY_MAX_LBA_EXT));
            } else
                sectors = *((uint32_t *)(ide_buffer + IDE_IDENTIFY_MAX_LBA));

            // FIXME: drive could report 0 sectors (empty qemu image)
            drives[drvs_cnt].last_sector = sectors ? sectors - 1 : 0;

            ide_strncpy(drives[drvs_cnt].model, (char const *)(ide_buffer + IDE_IDENTIFY_MODEL), 40);
            ide_strncpy(drives[drvs_cnt].serial, (char const *)(ide_buffer + IDE_IDENTIFY_SERIAL), 20);
            ide_strncpy(drives[drvs_cnt].firmware, (char const *)(ide_buffer + IDE_IDENTIFY_FIRMWARE), 8);

            uint8_t mode;

            if (drives[drvs_cnt].capabilities & IDE_PIO_SUPPORTED) {
                if (drives[drvs_cnt].command_sets & IDE_MODE_PIO_48)
                    mode = 2; // pio48
                else
                    mode = 1; // pio28
            } else
                mode = 0; // chs

            drives[drvs_cnt].mode = mode;
            drives[drvs_cnt].dma = drives[drvs_cnt].capabilities & IDE_DMA_SUPPORTED;

            printk("Found %s %s %s\n", (char const *[]){ "primary", "secondary" }[ch], (char const *[]){ "master", "slave" }[drv], (char const *[]){ "Unknown", "PATA", "SATA", "PATAPI", "SATAPI" }[type]);
            printk(" - Model      : %s\n", drives[drvs_cnt].model);
            printk(" - Signature  : %x\n", drives[drvs_cnt].signature); // %04x
            printk(" - Last sector: %u\n", drives[drvs_cnt].last_sector);
            printk(" - Serial     : %s\n", drives[drvs_cnt].serial);
            printk(" - Firmware   : %s\n", drives[drvs_cnt].firmware);
            printk(" - Mode       : %s\n", (char const *[]){ "CHS (Unsupported)", "LBA28", "LBA48" }[mode]);
            printk(" - DMA        : %s\n", drives[drvs_cnt].dma ? "supported" : "unsupported");

            ++drvs_cnt;
        }
    }
}

/**
 * ide_poll
*/

int ide_poll(uint8_t channel, int check_state) {
    for (uint32_t i = 0; i < 4; ++i)
        ide_read(channel, IDE_ALTSTATUS_REG);

    // TODO: add timeout
    while (ide_read(channel, IDE_STATUS_REG) & IDE_STATUS_BSY);

    if (check_state) {
        uint8_t status = ide_read(channel, IDE_STATUS_REG);

        if (status & IDE_STATUS_ERR)
            return -1; // error (read error register)
        else if (status & IDE_STATUS_DF)
            return -2; // device fault
        else if (!(status & IDE_STATUS_DRQ))
            return -3; // data request not ready
    }

    return 0;
}

/**
 * ide_read_blocks
*/

int ide_read_blocks(uint8_t minor, uint32_t lba, uint32_t count, char *buffer) {
    // count must be a positive number
    if (!count)
        return -1; // invalid parameter

    if (minor > 3)
        return -2; // invalid drive number
    
    struct ide_drive *drv = &drives[minor];

    if (((uint64_t)lba + count - 1) > (uint64_t)drv->last_sector)
        return -3; // out of disk space

    uint8_t mode = drv->mode;

    if (!mode)
        return -4; // chs mode not supported

    uint8_t h;
    uint8_t s = count;
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

        default:
            // chs and other modes not supported
            return -5;
    }

    uint8_t channel = drv->channel;
    
    while (ide_read(channel, IDE_STATUS_REG) & IDE_STATUS_BSY);

    ide_write(channel, IDE_HDDEVSEL_REG, 0xe0 | ((drv->drive & 0x01) << 4) | h);

    if (mode == 2) {
        ide_write(channel, IDE_SECCOUNT1_REG, 0);
        ide_write(channel, IDE_LBA3_REG, lbas[3]);
        ide_write(channel, IDE_LBA4_REG, lbas[4]);
        ide_write(channel, IDE_LBA5_REG, lbas[5]);
    }

    ide_write(channel, IDE_SECCOUNT0_REG, s);
    ide_write(channel, IDE_LBA0_REG, lbas[0]);
    ide_write(channel, IDE_LBA1_REG, lbas[1]);
    ide_write(channel, IDE_LBA2_REG, lbas[2]);

    ide_write(channel, IDE_COMMAND_REG, mode == 2 ? IDE_READ_PIO_EXT_CMD : IDE_READ_PIO_CMD);

    uint16_t *offset = (uint16_t *)buffer;
    uint16_t port = channels[channel].io_base;

    for (uint32_t i = 0; i < count; ++i) {
        if (ide_poll(channel, 1))
            return -6;

        asm volatile (
            "rep insw"
            :
            : "c" (256), "d" (port), "D" (offset)
            : "memory", "cc"
        );

        offset += 256;
    }

    return 0;
}

// function that is responsible for
// preparing hd for data transfer
static void do_requests(void) {
    struct ide_request *req = &requests[head];

    struct ide_drive *drv = &drives[req->drv];

    uint32_t lba = req->lba;
    uint32_t count = req->count;

    prdt.base = (uint32_t)req->buffer;
    prdt.bytes = req->count * 512;
    prdt.flags = PRD_EOT;

    // store address of prdt in bus master register
    outd(channels[drv->channel].busm_base + 4, (uint32_t)&prdt - KERNEL_VIRT_BASE);
    
    // set direction to read
    outb(channels[drv->channel].busm_base + 0, inb(channels[drv->channel].busm_base + 0) | 0x08);

    // clear interrupt and error bits
    outb(channels[drv->channel].busm_base + 2, 0x06);

    if (((uint64_t)lba + count - 1) > (uint64_t)drv->last_sector) {
        printk("out of disk");
        for(;;);
    }

    uint8_t mode = drv->mode;

    if (!mode) {
        printk("chs not supported");
        for(;;);
    }

    uint8_t h;
    uint8_t s = count;
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

        default:
            printk("invalid mode");
            for(;;);
    }

    uint8_t channel = drv->channel;
    
    while (ide_read(channel, IDE_STATUS_REG) & IDE_STATUS_BSY);

    ide_write(channel, IDE_HDDEVSEL_REG, 0xe0 | ((drv->drive & 0x01) << 4) | h);

    if (mode == 2) {
        ide_write(channel, IDE_SECCOUNT1_REG, 0);
        ide_write(channel, IDE_LBA3_REG, lbas[3]);
        ide_write(channel, IDE_LBA4_REG, lbas[4]);
        ide_write(channel, IDE_LBA5_REG, lbas[5]);
    }

    ide_write(channel, IDE_SECCOUNT0_REG, s);
    ide_write(channel, IDE_LBA0_REG, lbas[0]);
    ide_write(channel, IDE_LBA1_REG, lbas[1]);
    ide_write(channel, IDE_LBA2_REG, lbas[2]);

    ide_write(channel, IDE_COMMAND_REG, mode == 2 ? IDE_READ_DMA_EXT_CMD : IDE_READ_DMA_CMD);

    // set start/stop bit
    outb(channels[drv->channel].busm_base + 0, inb(channels[drv->channel].busm_base + 0) | 0x01);
}

__attribute__((interrupt)) static void primary_ata_hd_handler(struct interrupt_frame *frame) {
    acknowledge_irq(14);

    struct ide_request *req = &requests[head];

    struct ide_drive *drv = &drives[req->drv];

    // clear start/stop bit
    outb(channels[drv->channel].busm_base + 0, inb(channels[drv->channel].busm_base + 0) & ~0x01);

    // read status byte
    (void)inb(channels[drv->channel].busm_base + 2);

    // data is ready
    signal(&(req->sem));

    if (head == tail)
        head = tail = NONE_RID;
    else {
        head = req->next;

        do_requests();
    }
}

/**
 * ide_init
*/

int ide_init(struct pci_header *h) {
    struct pci_h_device *d = (struct pci_h_device *)h;

    uint8_t progif = d->h.h_prog_if;

    uint8_t ch0_mode = progif & IDE_CHANNEL_0_MODE;
    uint8_t ch0_switch = progif & IDE_CHANNEL_0_SWITCHING;
    uint8_t ch1_mode = progif & IDE_CHANNEL_1_MODE;
    uint8_t ch1_switch = progif & IDE_CHANNEL_1_SWITCHING;

    printk("        Channel 0 mode          : %s\n", ch0_mode ? "native" : "compatibility");
    printk("        Channel 0 mode switching: %s\n", ch0_switch ? "enabled" : "disabled");

    printk("        Channel 1 mode          : %s\n", ch1_mode ? "native" : "compatibility");
    printk("        Channel 1 mode switching: %s\n", ch1_switch ? "enabled" : "disabled");

    printk("        Bus mastering           : %s\n", progif & IDE_BUS_MASTERING ? "supported" : "unsupported");

    uint16_t io_base_0 = IDE_CHANNEL_0_IO_BASE;
    uint16_t ctrl_base_0 = IDE_CHANNEL_0_CTRL_BASE;

    if (ch0_mode) {
        io_base_0 += pci_fix_bar(d->d_bar0);
        ctrl_base_0 += pci_fix_bar(d->d_bar1);
    }
        
    uint16_t io_base_1 = IDE_CHANNEL_1_IO_BASE;
    uint16_t ctrl_base_1 = IDE_CHANNEL_1_CTRL_BASE;

    if (ch1_mode) {
        io_base_1 += pci_fix_bar(d->d_bar2);
        ctrl_base_1 += pci_fix_bar(d->d_bar3);
    }

    uint16_t busm_base_0 = 0;
    uint16_t busm_base_1 = 0;

    if (progif & IDE_BUS_MASTERING) {
        busm_base_0 += pci_fix_bar(d->d_bar4);
        busm_base_1 += busm_base_0 + 8;
    }

    install_interrupt_gate(0x76, &primary_ata_hd_handler);
    enable_irq(14);

    channels[IDE_CHANNEL_0].io_base = io_base_0;
    channels[IDE_CHANNEL_0].ctrl_base = ctrl_base_0;
    channels[IDE_CHANNEL_0].busm_base = busm_base_0;
    channels[IDE_CHANNEL_0].flags = 0; //IDE_IRQS_DISABLED; 

    channels[IDE_CHANNEL_1].io_base = io_base_1;
    channels[IDE_CHANNEL_1].ctrl_base = ctrl_base_1;
    channels[IDE_CHANNEL_1].busm_base = busm_base_1;
    channels[IDE_CHANNEL_1].flags = 0; //IDE_IRQS_DISABLED;

    // disable interrupts
    //ide_write(IDE_CHANNEL_0, IDE_CONTROL_REG, 0x02);
    //ide_write(IDE_CHANNEL_1, IDE_CONTROL_REG, 0x02);

    // enable irqs
    ide_write(IDE_CHANNEL_0, IDE_CONTROL_REG, 0x00);
    ide_write(IDE_CHANNEL_1, IDE_CONTROL_REG, 0x00);

    // identify drives
    ide_identify_drives();

    for (size_t i = 0; i < NREQ; ++i)
        requests[i].cmd = IDE_FREE;

    return 0;
}

/**
 * ide_add_request
*/

static void ide_add_request(uint8_t cmd, struct block_buffer *buffer) {
    struct ide_request *free_req;

    struct ide_request *requests_end = requests + NREQ;

    while (1) {
        for (free_req = requests; free_req < requests_end; ++free_req)
            if (free_req->cmd == -1)
                goto set_up_request;

        // TODO: wait for free requests (sleep)
    }

set_up_request:
    // set up request
}

void reqinfo(void) {
    unsigned long flags;

    save_flags(flags);
    cli();

    for (size_t i = 0; i < NREQ; ++i) {
        struct ide_request *req = &requests[i];

        printk("REQ #%u:\n LBA: %u\n CNT: %u\n CMD: %i\n\n", i, req->lba, req->count, req->cmd);
    }

    restore_flags(flags);
}

int ide_read_func(int minor, unsigned long lba, unsigned long count, char *buffer) {
    size_t index;

    // on single core there is no need
    // for the while loop but we can
    // keep it
    while (1) {
        wait(&reqs_sem);

        for (index = 0; index < NREQ; ++index)
            if (requests[index].cmd == IDE_FREE)
                goto prepare_req;
    }

prepare_req:
    struct ide_request *req = &requests[index];

    req->cmd = IDE_READ;
    req->drv = minor;
    req->lba = lba;
    req->count = count;
    req->buffer = buffer;
    req->sem.count = 0;
    req->sem.wait_list = NULL;
    req->next = NONE_RID;

    // just insert - no sorting (implement latter)
    if (head == NONE_RID && tail == NONE_RID) {
        req->previous = NONE_RID;

        head = tail = index;

        do_requests();
    } else {
        req->previous = tail;
        requests[tail].next = index;
        tail = index;
    }

    wait(&(req->sem));
    
    // free request slot
    req->cmd = IDE_FREE;

    signal(&reqs_sem);
    return 0;
}