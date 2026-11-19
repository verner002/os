/**
 * @file pci.c
 * @author verner002
 * @date 04/09/2025
*/

#include <stddef.h>

#include <kernel/pci.h>
#include <kernel/io.h>
#include <kernel/printk.h>

#define sizeofarray(x) (sizeof(x) / sizeof(*x))

#define PCI_CONFIG_ADDRESS              0x0cf8
#define PCI_CONFIG_DATA                 0x0cfc

#define PCI_CONFIG_CYCLE_ENABLED        0x80000000
#define PCI_MULTIFUNCTION               0x80
#define PCI_HEADER_TYPE                 0x7f
#define PCI_HEADER_TYPE_GENERAL_DEVICE  0x00

extern int ide_init(struct pci_header *h);
//extern int e1000_init(struct pci_header *h);

struct pci_class {
    uint8_t c_id;
    char *c_name;
    uint32_t c_sclasses_cnt;
    struct pci_subclass const *c_sclasses;
};

struct pci_subclass {
    uint8_t s_id;
    char *s_name;
    int32_t (* s_init)(struct pci_header *h);
};

/*struct pci_device {
    uint8_t d_bus_id;
    uint8_t d_dev_id;
    uint8_t d_func_id;
    char const *d_name;
    struct pci_header *d_header; // should be casted based on header_type
    int32_t (*d_init)(struct __bus *, struct pci_header *);
};*/

static struct pci_subclass const unclassified[] = {
    { 0, "Non-VGA compatible device", NULL },
    { 1, "VGA compatible device", NULL }
};

static struct pci_subclass const mass_storage_controller[] = {
    { 1, "IDE controller", &ide_init },
    { 2, "Floppy disk controller", NULL },
    { 5, "ATA controller", NULL /*&ide_init*/ },
    { 6, "SATA controller", NULL /*&ide_init*/ }
};

static struct pci_subclass const network_controller[] = {
    { 0, "Ethernet controller", NULL /*&e1000_init*/ }
};

static struct pci_subclass const display_controller[] = {
    { 0, "VGA compatible controller", NULL }
};

static struct pci_subclass const bridge[] = {
    { 0, "Host bridge", NULL },
    { 1, "ISA bridge", NULL },
    { 2, "EISA bridge", NULL },
    { 3, "MCA bridge", NULL },
    { 4, "PCI-to-PCI bridge", NULL },
    { 9, "PCI-to-PCI bridge", NULL }
};

static struct pci_subclass const base_system_peripheral[] = {
    { 0, "Programmable interrupt controller", NULL },
    { 1, "DMA controller", NULL },
    { 2, "Programmable interval timer", NULL },
    { 3, "RTC controller", NULL },
    { 4, "PCI hot-plug controller", NULL }
};

static struct pci_subclass const input_device_controller[] = {
    { 0, "Keyboard controller", NULL },
    { 2, "Mouse controller", NULL }
};

static struct pci_subclass const processor[] = {
    { 0, "Intel 386", NULL },
    { 1, "Intel 486", NULL },
    { 2, "Intel Pentium", NULL },
    { 3, "Intel Pentium Pro", NULL }
};

static struct pci_subclass const serial_bus_controller[] = {
    { 3, "USB controller", NULL }
};

static struct pci_class const classes[] = {
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
 * pci_config_read
*/

uint32_t pci_config_read(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset) {
    if (dev > 31 || func > 7 || offset & 3)
        return 0xffffffff; // invalid addresses

    // config space access mechanism pci 2.0+
    outd(PCI_CONFIG_ADDRESS,
        ((uint32_t)bus << 16) |
        ((uint32_t)(dev & 0x1f) << 11) |
        ((uint32_t)(func & 0x07) << 8) |
        ((uint32_t)(offset & 0xfc) << 0) |
        PCI_CONFIG_CYCLE_ENABLED
    );

    return ind(PCI_CONFIG_DATA);
}

/**
 * pci_fix_bar
*/

uint32_t pci_fix_bar(uint32_t bar) {
    return bar ? (bar & (bar & 0x00000001 ? 0xfffffffc : 0xfffffff0)) : 0x00000000;
}

/**
 * pci_init
*/

int pci_init(void) {
    for (int bus_i = 0; bus_i < 256; ++bus_i) {
        for (int dev_i = 0; dev_i < 32; ++dev_i) {
            // TODO: initialize struct __dev here (class for device
            //  subclass for subdevices, when calling init pass
            //  subdevice __dev struct to it, it will set additional
            //  information), read function 0 info here

            for (int func_i = 0; func_i < 8; ++func_i) {
                struct pci_header dev_header;

                for (uint32_t i = 0; i < sizeof(struct pci_header) / sizeof(uint32_t); ++i)
                    ((uint32_t *)&dev_header)[i] = pci_config_read(
                        (unsigned char)bus_i,
                        (unsigned char)dev_i,
                        (unsigned char)func_i,
                        sizeof(uint32_t) * (unsigned char)i
                    );

                // RFC: do we have to continue with scanning
                //  another functions if no device on func 0
                //  was found?
                if (dev_header.h_vendor == 0xffff)
                    continue;

                char *dev_name;
                int (* dev_init)(struct pci_header *h) = NULL;

                struct pci_class const *dev_class = NULL;

                // use binary search? (array is ordered)
                for (size_t i = 0; i < sizeofarray(classes); ++i)
                    if (classes[i].c_id == dev_header.h_class) {
                        dev_class = &classes[i];
                        break;
                    }

                if (dev_class) {
                    struct pci_subclass const *subclasses = dev_class->c_sclasses;
                    struct pci_subclass const *dev_subclass = NULL;

                    // use binary search? (array is ordered)
                    for (size_t i = 0; i < dev_class->c_sclasses_cnt; ++i)
                        if (subclasses[i].s_id == dev_header.h_subclass) {
                            dev_subclass = &subclasses[i];
                            break;
                        }

                    //printk("pci: info:    subclass [%s]\n", dev_subclass ? dev_subclass->s_name : "unknown");
                    dev_name = dev_subclass ? dev_subclass->s_name : dev_class->c_name;
                    
                    if (dev_subclass)
                        dev_init = dev_subclass->s_init;
                } else
                    dev_name = "Unknown";

                printk("%x:%x.%u %s\n", bus_i, dev_i, func_i, dev_name); // %02x:%02x.%u %s\n

                struct pci_header *init_header = NULL;
                uint8_t header_type = dev_header.h_header_type & PCI_HEADER_TYPE;

                switch (header_type) {
                    case PCI_HEADER_TYPE_GENERAL_DEVICE: {
                        struct pci_h_device pci_device;

                        pci_device.h = dev_header;

                        for (uint32_t i = sizeof(struct pci_header) / sizeof(uint32_t); i < sizeof(struct pci_h_device) / sizeof(uint32_t); ++i)
                            ((uint32_t *)&pci_device)[i] = pci_config_read(bus_i, dev_i, func_i, sizeof(uint32_t) * i);
                        
                        init_header = (struct pci_header *)&pci_device;
                        break;
                    }

                    default:
                        printk("pci: warning: unknown header type %u\n", header_type);
                        break;
                }

                if (dev_init && init_header) {
                    outd(PCI_CONFIG_ADDRESS,
                        ((uint32_t)bus_i << 16) |
                        ((uint32_t)(dev_i & 0x1f) << 11) |
                        ((uint32_t)(func_i & 0x07) << 8) |
                        ((uint32_t)(4 & 0xfc) << 0) |
                        PCI_CONFIG_CYCLE_ENABLED
                    );
                
                    // RFC: bus mastering must be enabled for IDE? (i'm pretty sure it must)
                    uint16_t command =  init_header->h_command;
                    command &= ~(1 << 10); // enable interrupts
                    command |= 4; // enable bus master

                    outd(PCI_CONFIG_DATA, ((uint32_t)init_header->h_status << 16) | command);
                    
                    dev_init(init_header);
                }

                if (!(dev_header.h_header_type & PCI_MULTIFUNCTION))
                    break;
            }
        }
    }

    return 0;
}