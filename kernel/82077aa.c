/**
 * @file 82077aa.c
 * @author verner002
 * @date 05/11/2025
*/

#include <stddef.h>
#include <string.h>

#include <kernel/82077aa.h>
#include <kernel/printk.h>
#include <kernel/io.h>
#include <kernel/irqs.h>
#include <kernel/8237a.h>
#include <kernel/cpu.h>
#include <kernel/cmos.h>
#include <kernel/idt.h>
#include <kernel/process.h>

#define FDC_STATUS_REGISTER_A                   0x03f0 // read-only
#define FDC_STATUS_REGISTER_B                   0x03f1 // read-only
#define FDC_DIGITAL_OUTPUT_REGISTER             0x03f2 // write-only
#define FDC_TAPE_DRIVE_REGISTER                 0x03f3
#define FDC_MAIN_STATUS_REGISTER                0x03f4 // read
#define FDC_DATARATE_SELECT_REGISTER            0x03f4 // write
#define FDC_DATA_FIFO                           0x03f5
#define FDC_DIGITAL_INPUT_REGISTER              0x03f7 // write
#define FDC_CONFIGURATION_CONTROL_REGISTER      0x03f7 // read

#define FDC_OUTB_TIMEOUT        500
#define FDC_INB_TIMEOUT         500
#define FDC_IRQ6_TIMEOUT        500

// parameters for 1.44 MB floppy
#define DATARATE                500000
#define HEAD_ASSEMBLY           8 // time the controller should wait for the head assembly to move between successive cylinders
#define HEAD_ACTIVATION         30 // time the controller should wait between activating a head and actually performing a read/write, set to 0 (maximum in any mode)?
#define HEAD_DEACTIVATION       240 // time the controller should wait before deactivating the head

typedef enum _drive_type DRIVE_TYPE;
typedef enum _fdc_command FDC_COMMAND;
typedef enum __fdc_command_extension FDC_COMMAND_EXTENSION;
typedef enum __fdc_gap3_length FDC_GAP3_LENGTH;
typedef enum __fdc_sector_dtl FDC_SECTOR_DTL;
typedef struct _drive DRIVE;

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

struct _drive {
    int id;
    DRIVE_TYPE type; // drive type from cmos
    int motor_on;
};

static struct {
    //uint8_t driveSelect; -- 0 for now
    int op_mode; // clear = reset mode, set = normal mode
    int irqs_enabled;
    int irq_received; // set by irq6 handler
    //DRIVE master;
    //DRIVE slave; -- only master for now
    int motor_on;
    DRIVE_TYPE type;
} fdc; //controller; -- fdc for now

static size_t motor_off_counter;
static struct semaphore motor_semaphore;

#define NREQ            1 //32

#define NONE_RID        -1
#define MIN_RID         0
#define MAX_RID         (NREQ - 1)

typedef int rid_t;

#define FDC_FREE        -1
#define FDC_READ        0
#define FDC_WRITE       1

static rid_t req_head = NONE_RID, req_tail = NONE_RID;

static struct semaphore reqs_sem = {
    .count = NREQ,
    .wait_list = NULL
};

static struct fdc_request {
    int8_t cmd; // command (FREE=-1, READ=0, WRITE=1)
    uint8_t drv; // drive number
    uint32_t lba; // starting block
    uint32_t count; // number of blocks
    char *buffer; // pointer to buffer
    struct semaphore sem; // buffer semaphore
    rid_t previous; // previous request index
    rid_t next; // next request index
} requests[NREQ];

extern void *fdc_buffer;
extern size_t fdc_buffer_size;

static void fdc_update_dor(void) {
    outb(FDC_DIGITAL_OUTPUT_REGISTER, (fdc.motor_on << 4) | (fdc.irqs_enabled << 3) | (fdc.op_mode << 2));
}

static void fdc_enter_reset_mode(void) {
    fdc.irqs_enabled = 0;
    fdc.op_mode = 0; // reset mode
    fdc_update_dor();
}

static void fdc_exit_reset_mode(void) {
    fdc.irqs_enabled = 1;
    fdc.op_mode = 1; // normal mode
    fdc_update_dor();
}

static void fdc_turn_motor_off(void) {
    fdc.motor_on = 0;
    fdc_update_dor();
}

static void fdc_turn_motor_on(void) {
    fdc.motor_on = 1;
    fdc_update_dor();
}

static int fdc_outb(uint8_t value) {
    for (uint32_t i = 0; i < FDC_OUTB_TIMEOUT; ++i) {
        if ((inb(FDC_MAIN_STATUS_REGISTER) & 0xc0) == 0x80) {
            outb(FDC_DATA_FIFO, value);
            return 0;
        }

        io_wait();
    }

    return 1;
}

static int fdc_inb(uint8_t *value) {
    for (uint32_t i = 0; i < FDC_INB_TIMEOUT; ++i) {
        if ((inb(FDC_MAIN_STATUS_REGISTER) & 0xc0) == 0xc0) {
            *value = inb(FDC_DATA_FIFO);
            return 0;
        }

        io_wait();
    }

    return 1;
}

static int fdc_wait_for_irq6(void) {
    for (size_t i = 0; i < FDC_IRQ6_TIMEOUT; ++i) {
        if (fdc.irq_received)
            return 0;

        io_wait();
    }

    return 1;
}

static int fdc_software_reset(void) {
    fdc.irq_received = 0;

    fdc_enter_reset_mode();
    io_wait(); // 4 us should be enough
    fdc_exit_reset_mode();

    outb(FDC_CONFIGURATION_CONTROL_REGISTER, 0x00); // 500 kbit/s for 1.44MB 3.5'
    
    return fdc_wait_for_irq6();
}

static int fdc_sense_interrupt(uint8_t *st0, uint8_t *pcn) {
    int error = fdc_outb(FDC_COMMAND_SENSE_INTERRUPT);

    if (error)
        goto result;

    error = fdc_inb(st0);

    if (error)
        goto result;

    error = fdc_inb(pcn);

result:
    return error;
}

static int fdc_recalibrate(void) {
    for (size_t i = 0; i < 3; ++i) {
        fdc.irq_received = 0;

        if (fdc_outb(FDC_COMMAND_RECALIBRATE))
            continue;

        // drive 0
        if (fdc_outb(0))
            continue;
        
        if (fdc_wait_for_irq6())
            continue;

        uint8_t st0, pcn;
        
        if (fdc_sense_interrupt(&st0, &pcn))
            continue;

        return pcn ? 1 : 0;
    }

    return 1;
}

int fdc_reset(void) {
    int error;

    for (size_t i = 0; i < 3; ++i) {
        if (fdc_software_reset())
            continue;

        error = 0;

        // send sense interrupt to each of four drives
        for (size_t j = 0; j < 4; ++j) {
            uint8_t st0, pcn;

            if (fdc_sense_interrupt(&st0, &pcn)) {
                error = 1;
                break;
            }
        }

        if (error)
            continue;

        if (fdc_outb(FDC_COMMAND_CONFIGURE))
            continue;
        
        if (fdc_outb(0x00))
            continue;

        uint8_t const implied_seek_enabled = 1;
        uint8_t const fifo_disabled = 0;
        uint8_t const polling_disabled = 1;
        uint8_t const threshold = 8 - 1; // value - 1
        
        if (fdc_outb((implied_seek_enabled << 6) | (fifo_disabled << 5) | (polling_disabled << 4) | threshold))
            continue;

        // precompensation = 0
        if (fdc_outb(0x00))
            continue;
        
        if (fdc_outb(FDC_COMMAND_SPECIFY))
            continue;

        uint8_t srt = 16 - (HEAD_ASSEMBLY * DATARATE / 500000);
        uint8_t hlt = HEAD_ACTIVATION * DATARATE / 1000000;
        uint8_t hut = HEAD_DEACTIVATION * DATARATE / 8000000;
        uint8_t ndma = 0; // use dma

        if (fdc_outb((srt << 4) | (hut & 0x0f)))
            continue;

        if (fdc_outb((hlt << 1) | ndma))
            continue;

        fdc_turn_motor_on();
        delay_ms(300);
        
        if (fdc_recalibrate()) {
            fdc_turn_motor_off();
            continue;
        }

        fdc_turn_motor_off();
        return 0;
    }

    // motor is turned off
    return 1;
}

int fdc_seek(uint8_t head, uint8_t cylinder) {
    if (head & 0xc0)
        return 1;

    for (size_t i = 0; i < 3; ++i) {
        fdc.irq_received = 0;

        if (fdc_outb(FDC_COMMAND_SEEK))
            continue;

        if (fdc_outb((head << 2) | 0))
            continue;

        if (fdc_outb(cylinder))
            continue;

        if (fdc_wait_for_irq6())
            continue;

        uint8_t st0, pcn;

        if (fdc_sense_interrupt(&st0, &pcn))
            continue;

        return pcn == cylinder ? 0 : 1;
    }

    return 1;
}

#define DMA_START_ADDRESS_REGISTER_2            0x0004
#define DMA_COUNT_REGISTER_2                    0x0005
#define DMA_SINGLE_CHANNEL_MASK_REGISTER_03     0x000a
#define DMA_MODE_REGISTER_03                    0x000b
#define DMA_FLIP_FLOP_RESET_REGISTER_03         0x000c
#define DMA_CHANNEL_2_PAGE_ADDRESS_REGISTER     0x0081

static int fdc_dma_init(uint32_t buffer, uint16_t size) {
    if (buffer & 0xff000000)
        return 1;

    unsigned long flags;
    
    save_flags(flags);
    cli();

    outb(DMA_SINGLE_CHANNEL_MASK_REGISTER_03, 0x06); // mask on, channel 2
    io_wait();

    outb(DMA_FLIP_FLOP_RESET_REGISTER_03, 0x00); // reset flip-flop
    io_wait();

    outb(DMA_START_ADDRESS_REGISTER_2, (uint8_t)buffer); // low address
    io_wait();

    buffer >>= 8;
    outb(DMA_START_ADDRESS_REGISTER_2, (uint8_t)buffer); // high address
    io_wait();

    buffer >>= 8;
    outb(DMA_CHANNEL_2_PAGE_ADDRESS_REGISTER, (uint8_t)buffer); // extra address
    io_wait();

    outb(DMA_FLIP_FLOP_RESET_REGISTER_03, 0x00); // reset flip-flop
    io_wait();

    --size;
    outb(DMA_COUNT_REGISTER_2, (uint8_t)size); // 0xff, number of bytes in a 3.5' floppy track
    io_wait();

    size >>= 8;
    outb(DMA_COUNT_REGISTER_2, (uint8_t)size); // 0x23, number of bytes in a 3.5' floppy track
    io_wait();

    outb(DMA_SINGLE_CHANNEL_MASK_REGISTER_03, 0x02); // mask off, channel 2
    io_wait();
    
    restore_flags(flags);
    return 0;
}

static void fdc_dma_prepare_read(void) {
    unsigned long flags;
    
    save_flags(flags);
    cli();

    outb(DMA_SINGLE_CHANNEL_MASK_REGISTER_03, 0x06); // mask on, channel 2
    io_wait();

    /**
     * mod=01
     * idec=0
     * auto=1
     * tra=01
     * sel=10
    */
    outb(DMA_MODE_REGISTER_03, 0x56); // 0x46 without autoinit
    io_wait();

    outb(DMA_SINGLE_CHANNEL_MASK_REGISTER_03, 0x02); // mask off, channel 2
    io_wait();
    
    restore_flags(flags)
}

int fdc_read_sector(uint8_t cylinder, uint8_t head, uint8_t sector, uint32_t buffer) {    
    outb(FDC_CONFIGURATION_CONTROL_REGISTER, 0x00); // 500 kbit/s for 1.44MB 3.5'

    for (size_t i = 0; i < 3; ++i) {
        fdc_seek(head, cylinder);

        for (uint32_t j = 0; j < 3; ++j) {
            if (fdc_dma_init(buffer, 512))
                return 1;

            fdc_dma_prepare_read();

            fdc.irq_received = 0;

            if (fdc_outb(FDC_COMMAND_READ_DATA | FDC_COMMAND_EXTENSION_MULTITRACK /*| FDC_COMMAND_EXTENSION_SKIP*/ | FDC_COMMAND_EXTENSION_DENSITY))
                return 1;

            if (fdc_outb((head << 2) | 0))
                return 1;

            if (fdc_outb(cylinder))
                return 1;

            if (fdc_outb(head))
                return 1;

            if (fdc_outb(sector))
                return 1;

            if (fdc_outb(FDC_SECTOR_DTL_512))
                return 1;

            if (fdc_outb(FDC_SECTORS_PER_TRACK_3_5))
                return 1;

            if (fdc_outb(FDC_GAP3_LENGTH_3_5))
                return 1;

            if (fdc_outb(0xff))
                return 1;
            
            return 0;
        }

        fdc_recalibrate();
    }

    return 1;
}

int fdc_read_sectors(unsigned long lba, unsigned long count, void *buffer) {
    //wait(&motor_semaphore);

    uint8_t cylinder, head, sector;

    motor_off_counter = 0;

    if (!fdc.motor_on) {
        fdc_turn_motor_on();
        delay_ms(300);
    }

    int result = 0;

    for (size_t i = 0; i < count; ++i, ++lba, buffer += 512) {
        cylinder = lba / (2 * 18);
        head = (lba / 18) % 2;
        sector = (lba % 18) + 1;

        result = fdc_read_sector(cylinder, head, sector, (uint32_t)fdc_buffer);

        if (result)
            break;

        memcpy(buffer, fdc_buffer, 512);
    }

    //signal(&motor_semaphore);
    fdc_turn_motor_off();
    return result;
}

/*char const *get_drive_type_string(int minor) {
    static char const *names[] = {
        "No drive",
        "360 KB 5.25'",
        "1.2 MB 5.25'",
        "720 KB 3.5'",
        "1.44 MB 3.5'",
        "2.88 MB 3.5'"
    };

    if (minor >= ndrv)
        return "No drive";

    int type = drvtab[minor];

    if (type >= (sizeof(names) / sizeof(char const *)))
        return "Unknown";

    return names[type];
}*/

__attribute__((interrupt)) static void fdc_seek_handler(struct interrupt_frame *frame);
__attribute__((interrupt)) static void fdc_read_handler(struct interrupt_frame *frame);

static void do_requests(void) {
    struct fdc_request *req = &requests[req_head];

    uint32_t lba = req->lba;

    uint8_t cylinder = lba / (2 * 18);
    uint8_t head = (lba / 18) % 2;

    if (!fdc.motor_on) {
        fdc_turn_motor_on();
        //delay_ms(300);
    }

    outb(FDC_CONFIGURATION_CONTROL_REGISTER, 0x00); // 500 kbit/s for 1.44MB 3.5'

    install_interrupt_gate(0x26, &fdc_seek_handler);
    
    fdc_outb(FDC_COMMAND_SEEK);
    fdc_outb((head << 2) | 0);
    fdc_outb(cylinder);
}

__attribute__((interrupt)) static void fdc_handler(struct interrupt_frame *frame) {
    acknowledge_irq(6);

    fdc.irq_received = 1;
}

__attribute__((interrupt)) static void fdc_seek_handler(struct interrupt_frame *frame) {
    acknowledge_irq(6);

    struct fdc_request *req = &requests[req_head];

    uint32_t lba = req->lba;

    uint8_t cylinder = lba / (2 * 18);
    uint8_t head = (lba / 18) % 2;
    uint8_t sector = (lba % 18) + 1;

    uint8_t st0, pcn;

    if (fdc_sense_interrupt(&st0, &pcn)) {
        printk("sense int failed\n");
        for (;;);
    }

    if (pcn != cylinder) {
        printk("seek failed\n");
        for (;;);
    }

    if (fdc_dma_init((uint32_t)fdc_buffer, 512)) {
        printk("fdc dma init failed\n");
        for (;;);
    }

    install_interrupt_gate(0x26, &fdc_read_handler);
    
    fdc_dma_prepare_read();

    fdc_outb(FDC_COMMAND_READ_DATA | FDC_COMMAND_EXTENSION_MULTITRACK /*| FDC_COMMAND_EXTENSION_SKIP*/ | FDC_COMMAND_EXTENSION_DENSITY);

    fdc_outb((head << 2) | 0);

    fdc_outb(cylinder);
    fdc_outb(head);
    fdc_outb(sector);

    fdc_outb(FDC_SECTOR_DTL_512);
    fdc_outb(FDC_SECTORS_PER_TRACK_3_5);
    fdc_outb(FDC_GAP3_LENGTH_3_5);

    fdc_outb(0xff);
}

__attribute__((interrupt)) static void fdc_read_handler(struct interrupt_frame *frame) {
    acknowledge_irq(6);

    struct fdc_request *req = &requests[req_head];

    uint8_t st0;

    if (fdc_inb(&st0)) {
        printk("read st0 failed\n");
        //fdc_recalibrate();
        do_requests();
        return;
    }

    if ((st0 & 0xc0) != 0x00) {
        printk("invalid st0 value\n");
        //fdc_recalibrate();
        do_requests();
        return;
    }

    int error = 0;
    uint8_t result[6];

    for (size_t k = 0; k < 6; ++k) {
        if (fdc_inb(&result[k])) {
            error = 1;
            break;
        }
    }

    if (error) {
        printk("read status failed\n");
        //fdc_recalibrate();
        do_requests();
        return;
    }

    memcpy(req->buffer, fdc_buffer, 512);
    
    // data is ready
    signal(&(req->sem));

    if (req_head == req_tail) {
        req_head = req_tail = NONE_RID;
        fdc_turn_motor_off();
    } else {
        req_head = req->next;

        do_requests();
    }
}

static void fdc_daemon(void) {
    for (;;) {
        while (!fdc.motor_on);

        for (;;) {
            delay_ms(1000);

            wait(&motor_semaphore);

            if (++motor_off_counter >= 5)
                break;

            signal(&motor_semaphore);
        }

        motor_off_counter = 0;
        fdc_turn_motor_off();
        signal(&motor_semaphore);
    }
}

int fdc_init(void) {
    for (size_t i = 0; i < NREQ; ++i)
        requests[i].cmd = FDC_FREE;
    
    motor_off_counter = 0;

    motor_semaphore.count = 1;
    motor_semaphore.wait_list = NULL;

    uint8_t drives = cmos_read(CMOS_FLOPPY_TYPE_REGISTER);

    fdc.type = (DRIVE_TYPE)(drives >> 0x04);
    //fdc.slave.type = (DRIVE_TYPE)(drives & 0x0f);

    if (!fdc.type)
        return 0;
    
    install_interrupt_gate(0x26, &fdc_handler);
    enable_irq(6);

    if (fdc_reset()) {
        disable_irq(6);
        return 1;
    }

    /*int pid = create(&fdc_daemon, 1, 4096);

    if (pid < 0) {
        disable_irq(6);
        return 1;
    }*/

    return 0;
}

int fdc_read_func(int minor, unsigned long lba, unsigned long count, void *buffer) {
    size_t index;

    // on single core there is no need
    // for the while loop but we can
    // keep it
    while (1) {
        wait(&reqs_sem);

        for (index = 0; index < NREQ; ++index)
            if (requests[index].cmd == FDC_FREE)
                goto prepare_req;
    }

prepare_req:
    struct fdc_request *req = &requests[index];

    req->cmd = FDC_READ;
    req->drv = minor;
    req->lba = lba;
    req->count = count;
    req->buffer = buffer;
    req->sem.count = 0;
    req->sem.wait_list = NULL;
    req->next = NONE_RID;

    // just insert - no sorting (implement latter)
    if (req_head == NONE_RID && req_tail == NONE_RID) {
        req->previous = NONE_RID;

        req_head = req_tail = index;

        do_requests();
    } else {
        req->previous = req_tail;
        requests[req_tail].next = index;
        req_tail = index;
    }

    wait(&(req->sem));
    // free request slot
    req->cmd = FDC_FREE;
    signal(&reqs_sem);
    return 0;
}

/*int fdc_ioctl(int cmd, int minor, void *data) {
    switch (cmd) {
        case 0:
            break;

        case 1:
            break;
    }

    return -1;
}*/