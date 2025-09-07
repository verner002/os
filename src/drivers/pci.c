/**
 * @file pci.c
 * @author verner002
 * @date 04/09/2025
*/

#include "macros.h"
#include "drivers/pci.h"
#include "drivers/ports.h"
#include "kstdlib/stdio.h"

#define PCI_CONFIG_CYCLE_ENABLED 0x80000000
#define PCI_IDE_CHANNEL_0 0x00
#define PCI_IDE_CHANNEL_1 0x01
#define PCI_IDE_CHANNEL_0_NATIVE_MODE 0x01

struct __pci_header {
    uint16_t h_vendor;
    uint16_t h_id;
    uint16_t h_command;
    uint16_t h_status;
    uint8_t h_revision;
    uint8_t h_prog_if;
    uint8_t h_subclass;
    uint8_t h_class;
    uint8_t h_cache_line_sz;
    uint8_t h_latency_timer;
    uint8_t h_header_type;
    uint8_t h_bist;
};

struct __pci_device {
    struct __pci_header h;
    uint32_t d_bar0;
    uint32_t d_bar1;
    uint32_t d_bar2;
    uint32_t d_bar3;
    uint32_t d_bar4;
    uint32_t d_bar5;
    uint32_t d_cardbus_cis_ptr;
    uint16_t d_subsystem_vendor;
    uint16_t d_subsystem_id;
    uint32_t d_expansion_rom_base;
    uint8_t d_capabilities_ptr;
    uint8_t d_reserved0;
    uint8_t d_reserved1;
    uint8_t d_reserved2;
    uint32_t d_reserved3;
    uint8_t d_int_line;
    uint8_t d_int_pin;
    uint8_t d_min_grant;
    uint8_t d_max_latency;
};

int32_t __pci_init_ide(struct __pci_header h);

struct __class {
    uint8_t c_id;
    char *c_name;
    uint32_t c_sclasses_cnt;
    struct __subclass const *c_sclasses;
};

struct __subclass {
    uint8_t s_id;
    char *s_name;
    int32_t (* s_init)(struct __pci_header h);
};

static struct __subclass const unclassified[] = {
    { 0, "Non-VGA compatible device", NULL },
    { 1, "VGA compatible device", NULL }
};

static struct __subclass const mass_storage_controller[] = {
    { 1, "IDE controller", &__pci_init_ide },
    { 2, "Floppy disk controller", NULL },
    /*{ 5, "ATA controller", NULL },
    { 6, "SATA controller", NULL }*/
};

static struct __subclass const network_controller[] = {
    { 0, "Ethernet controller", NULL }
};

static struct __subclass const display_controller[] = {
    { 0, "VGA compatible controller", NULL }
};

static struct __subclass const bridge[] = {
    { 0, "Host bridge", NULL },
    { 1, "ISA bridge", NULL },
    { 2, "EISA bridge", NULL },
    { 3, "MCA bridge", NULL },
    { 4, "PCI-to-PCI bridge", NULL },
    { 9, "PCI-to-PCI bridge", NULL }
};

static struct __subclass const base_system_peripheral[] = {
    { 0, "Programmable interrupt controller", NULL },
    { 1, "DMA controller", NULL },
    { 2, "Programmable interval timer", NULL },
    { 3, "RTC controller", NULL },
    { 4, "PCI hot-plug controller", NULL }
};

static struct __subclass const input_device_controller[] = {
    { 0, "Keyboard controller", NULL },
    { 2, "Mouse controller", NULL }
};

static struct __subclass const processor[] = {
    { 0, "Intel 386", NULL },
    { 1, "Intel 486", NULL },
    { 2, "Intel Pentium", NULL },
    { 3, "Intel Pentium Pro"}
};

static struct __subclass const serial_bus_controller[] = {
    { 3, "USB controller", NULL }
};

static struct __class const classes[] = {
    { 0, "Unclassified", sizeofarray(unclassified), unclassified },
    { 1, "Mass storage controller", sizeofarray(mass_storage_controller), mass_storage_controller },
    { 2, "Network controller", sizeofarray(network_controller), network_controller },
    { 3, "Display controller", sizeofarray(display_controller), display_controller },
    { 6, "Bridge", sizeofarray(bridge), bridge },
    { 8, "Base system peripheral", sizeofarray(base_system_peripheral), base_system_peripheral },
    { 9, "Input device controller", sizeofarray(input_device_controller), input_device_controller },
    { 11, "Processor", sizeofarray(processor), processor },
    { 12, "Serial bus controller", sizeofarray(serial_bus_controller), serial_bus_controller }
};

/**
 * __pci_config_read
*/

uint32_t __pci_config_read(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    if (dev > 31 || func > 7)
        return 0xffffffff; // invalid addresses

    // config space access mechanism pci 2.0+
    __outd(PCI_CONFIG_ADDRESS,
        ((uint32_t)bus << 16) |
        ((uint32_t)(dev & 0x1f) << 11) |
        ((uint32_t)(func & 0x07) << 8) |
        ((uint32_t)(offset & 0xfc) << 0) |
        PCI_CONFIG_CYCLE_ENABLED
    );

    return __ind(PCI_CONFIG_DATA);
}

/**
 * __pci_init
*/

int32_t __init_pci(void) {
    for (uint16_t bus_i = 0; bus_i < 256; ++bus_i) {
        for (uint8_t dev_i = 0; dev_i < 32; ++dev_i) {
            struct __pci_header dev_header;

            for (uint32_t i = 0; i < sizeof(struct __pci_header) / sizeof(uint32_t); ++i)
                ((uint32_t *)&dev_header)[i] = __pci_config_read(bus_i, dev_i, 0, sizeof(uint32_t) * i);

            if (dev_header.h_vendor == 0xffff)
                continue;

            char *dev_name;
            int32_t (* dev_init)(struct __pci_header h) = (int32_t (*)(struct __pci_header))NULL;

            struct __class *dev_class = NULL;

            // use binary search? (array is ordered)
            for (uint32_t i = 0; i < sizeof(classes) / sizeof(struct __class); ++i)
                if (classes[i].c_id == dev_header.h_class) {
                    dev_class = &classes[i];
                    break;
                }

            if (dev_class) {
                struct __subclass *subclasses = dev_class->c_sclasses;
                struct __subclass *dev_subclass = NULL;

                // use binary search? (array is ordered)
                for (uint32_t i = 0; i < dev_class->c_sclasses_cnt; ++i)
                    if (subclasses[i].s_id == dev_header.h_subclass) {
                        dev_subclass = &subclasses[i];
                        break;
                    }

                //printk("pci: info:    subclass [%s]\n", dev_subclass ? dev_subclass->s_name : "unknown");
                dev_name = dev_subclass ? dev_subclass->s_name : dev_class->c_name;
                dev_init = dev_subclass->s_init;
            } else
                dev_name = "Unknown";

            printf("%02x:%02x.%u %s\n", bus_i, dev_i, 0, dev_name);

            if (dev_header.h_header_type & 0x80) {
                printk("pci: warning: multi-function devices not supported\n");
                continue;
            }

            struct __pci_header *pci_header;

            switch (dev_header.h_header_type & 0x7f) {
                case 0: {
                    struct __pci_device __device;
                    __device.h = dev_header;                    

                    for (uint32_t i = sizeof(struct __pci_header) / sizeof(uint32_t); i < sizeof(struct __pci_device) / sizeof(uint32_t); ++i)
                        ((uint32_t *)&__device)[i] = __pci_config_read(bus_i, dev_i, 0, sizeof(uint32_t) * i);
                    break;
                }
            }

            if (dev_init)
                dev_init(dev_header);
        }
    }

    return 0;
}

/**
 * __pci_init_ide_channel
*/

int32_t __pci_init_ide_channel(uint8_t prog_if) {
    switch (prog_if) {
        case PCI_IDE_CHANNEL_0: {
            break;
        }

        case PCI_IDE_CHANNEL_1: {
            break;
        }

        default:
            printk("pci: ide: unsupported channel %u\n", prog_if);
            return -1;
    }
}

/**
 * __pci_init_ide
*/

int32_t __pci_init_ide(struct __pci_header h) {
    uint8_t channel_0_mode = h.h_prog_if & 0x01;

    printf("        Channel 0 mode: %s\n", channel_0_mode ? "native" : "compatibility");
    printf("        Channel 0 mode switching: %s\n", h.h_prog_if & 0x02 ? "enabled" : "disabled");

    //if (channel_0_mode)

    printf("        Channel 1 mode: %s\n", h.h_prog_if & 0x04 ? "native" : "compatibility");
    printf("        Channel 1 mode switching: %s\n", h.h_prog_if & 0x08 ? "enabled" : "disabled");
}