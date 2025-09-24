/**
 * @file e1000.c
 * @author verner002
 * @date 24/09/2025
*/

#include "bool.h"
#include "kstdlib/stdio.h"
#include "drivers/ports.h"
#include "drivers/bus/pci.h"
#include "drivers/net/e1000.h"
#include "mm/vmm.h"

#define REG_EEPROM 0x0014

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

struct {
    uint32_t cmt;
    uint32_t base;
} e1000;

struct __attribute__((__packed__)) __e1000_rx_desc {
        volatile uint64_t addr;
        volatile uint16_t length;
        volatile uint16_t checksum;
        volatile uint8_t status;
        volatile uint8_t errors;
        volatile uint16_t special;
};

struct __attribute__((__packed__)) __e1000_tx_desc {
        volatile uint64_t addr;
        volatile uint16_t length;
        volatile uint8_t cso;
        volatile uint8_t cmd;
        volatile uint8_t status;
        volatile uint8_t css;
        volatile uint16_t special;
};

void __e1000_write(uint16_t address, uint32_t data) {
    switch (e1000.cmt) {
        case 0:
            __outd(e1000.base, address);
            __outd(e1000.base + 4, data);
            break;

        case 1:
            *(uint32_t *)(e1000.base + address) = data;
            break;
    }
}

uint32_t __e1000_read(uint16_t address) {
    switch (e1000.cmt) {
        case 0:
            __outd(e1000.base, address);
            return __ind(e1000.base + 4);

        case 1:
            return *(uint32_t *)(e1000.base + address);
    }

    return 0;
}

int32_t __init_e1000(struct __bus *b, struct __pci_header *h) {
    printf("        Initializing...\n");

    struct __pci_h_device *d = (struct __pci_h_device *)h;

    printf("        VENDOR_ID=%04x DEVICE_ID=%04x\n", d->h.h_vendor, d->h.h_id);

    if (d->h.h_vendor != 0x8086 || (d->h.h_id != 0x100e && d->h.h_id != 0x153a && d->h.h_id != 0x10ea))
        printf("        Probably not an Intel E1000 network card, may not work correctly\n");

    printf("        Communication mechanism : ");

    if (d->d_bar0 & 0x00000001) {
        // ports
        printf("IO\n");
        printf("        IO Base                 : %08x\n", d->d_bar0 & 0xfffffffc);

        e1000.cmt = 0;
        e1000.base = d->d_bar0 & 0xfffffffc;
    } else {
        // mmio
        if (d->d_bar0 & 0x00000006 != 0x00000000) {
            printf("Unsupported\n");
            return 1;
        }

        printf("MMIO\n");
        printf("        MMIO Base               : %08x\n", d->d_bar0 & 0xfffffff0);

        e1000.cmt = 1;
        e1000.base = d->d_bar0 & 0xfffffff0;

        if (__map_page(e1000.base & 0xfffff000, e1000.base & 0xfffff000, PAGE_READ_WRITE))
            printk("mapping failed\n");
    }

    __e1000_write(REG_EEPROM, 0x00000001);

    bool eeprom_present = FALSE;

    for (uint32_t i = 0; i < 1000; ++i) {
        if (__e1000_read(REG_EEPROM) & 0x00000010) {
            eeprom_present = TRUE;
            break;
        }
    }

    printf("        EEPROM                  : %s\n", eeprom_present ? "present" : "not present");
    return -1;
}