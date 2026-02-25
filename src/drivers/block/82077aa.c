/**
 * @file 82077aa.c
 * @author verner002
 * @date 05/11/2025
*/

#include "macros.h"
#include "drivers/block/82077aa.h"
#include "hal/dev.h"
#include "fs/file.h"

static struct {
    //uint8_t driveSelect; -- 0 for now
    bool operationMode; // clear = reset mode, set = normal mode
    bool irqsEnabled;
    bool irqReceived; // set by irq6 handler
    //DRIVE master;
    //DRIVE slave; -- only master for now
    bool motorOn;
    DRIVE_TYPE type;
} fdc; //controller; -- fdc for now
static bool motor_mutex;
static uint32_t motor_off_counter;

/**
 * __update_dor
*/

void __update_dor(void) {
    __outb(FDC_DIGITAL_OUTPUT_REGISTER, (fdc.motorOn << 4) | (fdc.irqsEnabled << 3) | (fdc.operationMode << 2));
}

/**
 * __fdc_enter_reset_mode
*/

void __fdc_enter_reset_mode(void) {
    fdc.irqsEnabled = false;
    fdc.operationMode = false; // reset mode
    __update_dor();
}

/**
 * __fdc_exit_reset_mode
*/

void __fdc_exit_reset_mode(void) {
    fdc.irqsEnabled = true;
    fdc.operationMode = true; // normal mode
    __update_dor();
}

/**
 * __fdc_turn_motor_off
*/

void __fdc_turn_motor_off(void) {
    fdc.motorOn = false;
    __update_dor();
}

/**
 * __fdc_turn_motor_on
*/

void __fdc_turn_motor_on(void) {
    fdc.motorOn = true;
    __update_dor();
}

/**
 * __fdc_outb
*/

void __fdc_outb(uint8_t v) {
    for (uint32_t i = 0; i < __FDC_OUTB_TIMEOUT; ++i) {
        if ((__inb(FDC_MAIN_STATUS_REGISTER) & 0xc0) == 0x80) {
            __outb(FDC_DATA_FIFO, v);
            return;
        }

        __delay_ms(1);
    }

    errno = ETIMEDOUT;
    return;
}

/**
 * __fdc_inb
*/

uint8_t __fdc_inb(void) {
    for (uint32_t i = 0; i < __FDC_INB_TIMEOUT; ++i) {
        if ((__inb(FDC_MAIN_STATUS_REGISTER) & 0xc0) == 0xc0) return __inb(FDC_DATA_FIFO);

        __delay_ms(1);
    }

    errno = ETIMEDOUT;
    return 0;
}

/**
 * __fdc_irq6_handler
*/

__attribute__((interrupt)) void __fdc_irq6_handler(struct __interrupt_frame *frame) {
    //__disable_interrupts();
    fdc.irqReceived = true;
    __send_eoi(0x06);
    //__enable_interrupts();
}

/**
 * __fdc_wait_for_irq6
*/

int32_t __fdc_wait_for_irq6(void) {
    //printk("\033[33mfdc:\033[37m Waiting for IRQ6... ");

    for (uint32_t i = 0; i < __FDC_IRQ6_TIMEOUT; ++i) { // wait for irq 6
        if (fdc.irqReceived) {
            //printf("Received\n");
            return 0;
        }

        __delay_ms(1);
    }

    //printf("Timeout\n");
    return -1;
}

/**
 * __fdc_software_reset
*/

static int32_t __fdc_software_reset(void) {
    fdc.irqReceived = false;

    __fdc_enter_reset_mode();
    __delay_ms(1); // 4 us would be enough
    __fdc_exit_reset_mode();

    __outb(FDC_CONFIGURATION_CONTROL_REGISTER, 0x00); // 500 kbit/s for 1.44MB 3.5'
    
    return __fdc_wait_for_irq6();
}

/**
 * __fdc_sense_interrupt
*/

int32_t __fdc_sense_interrupt(uint8_t *st0, uint8_t *pcn) {
    __fdc_outb(FDC_COMMAND_SENSE_INTERRUPT);

    if (errno) return -1;

    *st0 = __fdc_inb();

    if (errno) return -1;

    *pcn = __fdc_inb();

    if (errno) return -1;

    return 0;
}

/**
 * __fdc_reset
*/

int32_t __fdc_reset(void) {
    for (uint32_t i = 0; i < 3; ++i) {
        errno = 0; // reset errno
        
        if (__fdc_software_reset())
            continue;

        for (uint32_t j = 0; j < 4; ++j) { // send sense interrupt to each of four drives
            uint8_t st0, pcn;

            if (__fdc_sense_interrupt(&st0, &pcn)) {
                errno = -1;
                break;
            }

            /*printk("\033[33mfdc:\033[37m Reset result:\n");
            printk("..... ST0=%u\n", st0);
            printk("..... PCN=%u\n", pcn);*/
        }

        if (errno)
            continue;

        __fdc_outb(FDC_COMMAND_CONFIGURE);
        if (errno)
            continue;

        __fdc_outb(0x00);
        if (errno)
            continue;

        bool impliedSeekEnabled = true;
        bool fifoDisabled = false;
        bool drivePollingModeDisabled = true;
        uint8_t threshold = 8 - 1; // value - 1

        __fdc_outb((impliedSeekEnabled << 6) | (fifoDisabled << 5) | (drivePollingModeDisabled << 4) | threshold);
        if (errno)
            continue;

        __fdc_outb(0x00); // precompensation = 0
        if (errno)
            continue;

        __fdc_outb(FDC_COMMAND_SPECIFY);
        if (errno)
            continue;

        uint8_t srt = 16 - (HEAD_ASSEMBLY * DATARATE / 500000);
        uint8_t hlt = HEAD_ACTIVATION * DATARATE / 1000000;
        uint8_t hut = HEAD_DEACTIVATION * DATARATE / 8000000;
        bool ndma = false; // use dma

        __fdc_outb((srt << 4) | (hut & 0x0f));
        if (errno) continue;

        __fdc_outb((hlt << 1) | ndma);
        if (errno) continue;

        __fdc_turn_motor_on();
        __delay_ms(300);
        
        if (__fdc_recalibrate()) {
            __fdc_turn_motor_off();
            continue;
        }

        __fdc_turn_motor_off();
        return 0;
    }

    // motor is turned off
    return -1;
}

/**
 * __fdc_recalibrate
*/

int32_t __fdc_recalibrate(void) {
    for (uint32_t i = 0; i < 3; ++i) {
        fdc.irqReceived = false;

        __fdc_outb(FDC_COMMAND_RECALIBRATE);
        __fdc_outb(0); // drive 0
        
        if (__fdc_wait_for_irq6())
            continue;

        uint8_t st0, pcn;
        
        if (__fdc_sense_interrupt(&st0, &pcn))
            continue;

        /*printk("\033[33mfdc:\033[37m Recalibrate result:\n");
        printk("..... ST0=%u\n", st0);
        printk("..... PCN=%u\n", pcn);*/

        return pcn ? -1 : 0;
    }

    return -1;
}

/**
 * __fdc_seek
*/

int32_t __fdc_seek(uint32_t head, uint32_t cylinder) {
    for (uint32_t i = 0; i < 3; ++i) {
        fdc.irqReceived = false;

        __fdc_outb(FDC_COMMAND_SEEK);
        __fdc_outb((head << 2) | 0);
        __fdc_outb(cylinder);

        if (__fdc_wait_for_irq6()) continue;

        uint8_t st0, pcn;

        if (__fdc_sense_interrupt(&st0, &pcn)) continue;

        /*printk("\033[33mfdc:\033[37m Seek result:\n");
        printk("..... ST0=%u\n", st0);
        printk("..... PCN=%u\n", pcn);*/

        return pcn == cylinder ? 0 : -1;
    }

    return -1;
}

/**
 * __init_fdc_dma
*/

void __init_fdc_dma(uint32_t buffer, uint16_t size) {
    __disable_interrupts();
    __outb(DMA_SINGLE_CHANNEL_MASK_REGISTER_03, 0x06); // mask on, channel 2
    __wait_for_dma();

    __outb(DMA_FLIP_FLOP_RESET_REGISTER_03, 0x00); // reset flip-flop
    __wait_for_dma();

    __outb(DMA_START_ADDRESS_REGISTER_2, (uint8_t)buffer); // low address
    __wait_for_dma();
    buffer >>= 8;
    __outb(DMA_START_ADDRESS_REGISTER_2, (uint8_t)buffer); // high address
    __wait_for_dma();
    buffer >>= 8;
    __outb(DMA_CHANNEL_2_PAGE_ADDRESS_REGISTER, (uint8_t)buffer); // extra address
    __wait_for_dma();

    __outb(DMA_FLIP_FLOP_RESET_REGISTER_03, 0x00); // reset flip-flop
    __wait_for_dma();

    --size;
    __outb(DMA_COUNT_REGISTER_2, (uint8_t)size); // 0xff, number of bytes in a 3.5' floppy track
    __wait_for_dma();
    size >>= 8;
    __outb(DMA_COUNT_REGISTER_2, (uint8_t)size); // 0x23, number of bytes in a 3.5' floppy track
    __wait_for_dma();

    __outb(DMA_SINGLE_CHANNEL_MASK_REGISTER_03, 0x02); // mask off, channel 2
    __enable_interrupts();
}

/**
 * __fdc_dma_prepare_read
*/

void __fdc_dma_prepare_read(void) {
    __disable_interrupts();
    __outb(DMA_SINGLE_CHANNEL_MASK_REGISTER_03, 0x06); // mask on, channel 2
    __wait_for_dma();

    /**
     * mod=01
     * idec=0
     * auto=1
     * tra=01
     * sel=10
     */
    __outb(DMA_MODE_REGISTER_03, 0x56); // 0x46 without autoinit
    __wait_for_dma();

    __outb(DMA_SINGLE_CHANNEL_MASK_REGISTER_03, 0x02); // mask off, channel 2
    __enable_interrupts();
}

/**
 * __fdc_read_sector
*/

int32_t __fdc_read_sector(uint8_t cylinder, uint8_t head, uint8_t sector, uint32_t buffer) {    
    __outb(FDC_CONFIGURATION_CONTROL_REGISTER, 0x00); // 500 kbit/s for 1.44MB 3.5'

    for (uint32_t i = 0; i < 3; ++i) {
        __fdc_seek(head, cylinder);

        for (uint32_t j = 0; j < 3; ++j) {
            __init_fdc_dma(buffer, 512);
            __fdc_dma_prepare_read();

            fdc.irqReceived = false;
            __fdc_outb(FDC_COMMAND_READ_DATA | FDC_COMMAND_EXTENSION_MULTITRACK /*| FDC_COMMAND_EXTENSION_SKIP*/ | FDC_COMMAND_EXTENSION_DENSITY);
            if (errno)
                return -1;
            
            __fdc_outb((head << 2) | 0);
            if (errno)
                return -1;
            
            __fdc_outb(cylinder);
            if (errno)
                return -1;
            
            __fdc_outb(head);
            if (errno)
                return -1;
            
            __fdc_outb(sector);
            if (errno)
                return -1;
            
            __fdc_outb(FDC_SECTOR_DTL_512);
            if (errno)
                return -1;
            
            __fdc_outb(FDC_SECTORS_PER_TRACK_3_5);
            if (errno)
                return -1;
            
            __fdc_outb(FDC_GAP3_LENGTH_3_5);
            if (errno)
                return -1;
            
            __fdc_outb(0xff);
            if (errno)
                return -1;

            if (__fdc_wait_for_irq6())
                return -1;

            uint8_t st0 = __fdc_inb();

            if (errno || (st0 & 0xc0) != 0x00)
                continue;

            for (uint32_t i = 0; i < 6; ++i) {
                __fdc_inb();

                if (errno)
                    break;
            }

            if (errno)
                continue;
            
            return 0;
        }

        __fdc_recalibrate();
    }

    return -1;
}

/**
 * __fdc_read_sectors
*/

extern void *fdc_buffer;

int32_t __fdc_read_sectors(uint32_t lba, uint32_t count, uint32_t buffer) {
    void *user_buffer = (void *)buffer;
    buffer = (uint32_t)fdc_buffer;

    if ((buffer + count * 512 - 1) > 0xfffff) {
        printk("\033[33mfdc:\033[37m Buffer must resize within the first MiB.\n");
        return -1;
    }

    __mutex_lock(&motor_mutex);

    uint8_t cylinder, head, sector;

    motor_off_counter = 0;

    if (!fdc.motorOn) {
        __fdc_turn_motor_on();
        __delay_ms(300);
    }

    int32_t last_opcode;

    for (uint32_t i = 0; i < count; ++i) {
        cylinder = lba / (2 * 18);
        head = (lba / 18) % 2;
        sector = (lba % 18) + 1;

        last_opcode = __fdc_read_sector(cylinder, head, sector, buffer);

        ++lba;
        buffer += 512;

        if (last_opcode)
            break;
    }

    __mutex_unlock(&motor_mutex);

    memcpy(user_buffer, fdc_buffer, 512*count);
    return last_opcode;
}

/**
 * __get_drive_type_string
*/

char const*__get_drive_type_string(DRIVE drive) {
    static char const *names[] = {
        "No drive",
        "360 KB 5.25'",
        "1.2 MB 5.25'",
        "720 KB 3.5'",
        "1.44 MB 3.5'",
        "2.88 MB 3.5'"
    };

    if ((uint32_t)drive.type >= sizeofarray(names))
        return "Unknown";

    return names[(uint32_t)drive.type];
}

static int32_t __fdc_daemon(int argc, char **argv) {
    for (;;) {
        while (!fdc.motorOn);

        do {
            __mutex_lock(&motor_mutex);
            __delay_ms(1000);
            ++motor_off_counter;
            __mutex_unlock(&motor_mutex);
        } while (motor_off_counter < 5);

        __mutex_lock(&motor_mutex);
        motor_off_counter = 0;
        __fdc_turn_motor_off();
        __mutex_unlock(&motor_mutex);
    }
}

/**
 * __ioctl
*/

int32_t __ioctl(uint32_t cmd, uint8_t minor, void *data) {
    switch (cmd) {
        case 0:
            break;

        case 1:
            
            break;
    }
}

/**
 * __init_fdc
*/

int fdc_read_sectors(uint8_t minor, uint32_t lba, uint32_t count, char *buffer) {
    return __fdc_read_sectors(lba, count, (uint32_t)buffer);
}

int32_t __init_fdc(void) {
    //printk("Initializing FDC...\n");
    //printk("\033[33mfdc:\033[37m Detecting drives... ");

    uint8_t drives = __read_cmos_register(CMOS_FLOPPY_TYPE_REGISTER);

    fdc.type = (DRIVE_TYPE)(drives >> 0x04);
    //fdc.slave.type = (DRIVE_TYPE)(drives & 0x0f);

    //printf("Done\n");

    if (!fdc.type) {
        //printk("\033[33mfdc:\033[37m No drive found\n");
        __exit(0);
    }

    //printk("\033[33mfdc:\033[37m Preparing IRQ6 handler... ");

    __disable_interrupts();
    __idt_set_handler(0x26, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__fdc_irq6_handler);
    __enable_interrupts();
    __enable_irq(0x06); // irq6

    //printf("Done\n");

    if (__fdc_reset()) {
        //printk("\033[33mfdc:\033[37m Failed to initialize FDC\n");
        __disable_irq(0x06);
        return -1;
    }

    /*__mutex_lock(&motor_mutex);
    motor_off_counter = 0;
    __mutex_unlock(&motor_mutex);*/

    int32_t pid = __create_thread("fdc-daemon", (int32_t (*)(int argc, char **argv))&__fdc_daemon, THREAD_RING_0, THREAD_PRIORITY_HIGH, NULL);

    if (pid < 0)
        return -1;

    if (register_blk_driver(FLOPPY_MAJOR, &fdc_read_sectors)) {
        printk("failed to register fdc-driver\n");
        return -1;
    }

    // creates device file in /dev -> /dev/fd0
    if (register_blk_device(MAJMIN(FLOPPY_MAJOR, 0))) {
        printk("failed to register fd0\n");
        for(;;);
        return -1;
    }

    /*if (__dev_add(MAJMIN(FLOPPY_MAJOR, 0), "fd0", NULL, NULL, NULL))
        return -1;*/

    /*if (!__register_driver("fdc", FLOPPY_MAJOR, &fdc_driver_type))
        __exit(-1);*/

    return 0;
}