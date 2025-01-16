/**
 * Advanced Configuration & Power Interface
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "drivers/acpi.h"

/**
 * __xaddr2addr
*/

uint32_t __xaddr2addr(uint64_t xaddress, uint32_t *address, char const *exceeds) {
    uint32_t xaddress_h = xaddress >> 32; // NOTE: check if compiled correctly
    
    if (xaddress_h) {
        printk(exceeds);
        return xaddress_h;
    }

    *address = (uint32_t)xaddress;

    return xaddress_h;
}

/**
 * __find_rsdp
*/

RSDP *__find_rsdp(void *start, uint32_t size) {
    for (uint32_t i = 0; i < size; i += 16) {
        uint64_t *ptr = start + i;

        if (*ptr == ' RTP DSR') return (RSDP *)(ptr);
    }

    return NULL;
}

/**
 * __init_acpi
*/

uint32_t __init_acpi(void) {
    printk("Initializing ACPI...\n");
    printk("\033[33macpi:\033[37m Looking for RSDP in EBDA... ");

    RSDP *rsdp = __find_rsdp((void *)(*(uint16_t *)0x0000040e << 4), 1024); // 40:0e - ebda base >> 4 (or lpt 4 address on non-ps/2 systems!!!, FIXME???)

    if (!rsdp) {
        printf("Not found\n");
        printk("\033[33macpi:\033[37m Looking for RSDP in MBA... ");

        rsdp = __find_rsdp((void *)0x000e0000, 131072);

        if (!rsdp) {
            printf("Not found\n");
            return -1;
        }
    }

    printf("Found\n");
    printk("\033[33macpi:\033[37m Calculating RSDP checksum... ");

    uint8_t checksum = 0; //  we only want the last byte (uint16_t would enough for whole rsdp checksum)
    uint8_t *rsdp_bytes = (uint8_t *)rsdp;

    for (unsigned int i = 0; i < sizeof(RSDP); ++i) checksum += rsdp_bytes[i];
    
    if (checksum) {
        // TODO: print error or something...?
        printf("Invalid\n");
        return -1;
    }

    printf("Valid\n");

    if (rsdp->revision) {
        printk("\033[33macpi:\033[37m \033[96mACPI version 2.0-6.1 => using XSDT\033[37m\n");

        //__parse_xsdt(rsdp);
    } else {
        printk("\033[33macpi:\033[37m \033[96mACPI version 1.0 => using RSDT\033[37m\n");

        /**
         * FIXME:
         *  paging is enabled, i can't access given addresses
         *  check if address is mapped, if not use static paging
         *  table to map the memory (paging directory for kernel
         *  passed by loader)
        */
        
        STD_HEADER *rsdt = (STD_HEADER *)rsdp->rsdt_base;

        printk("\033[33macpi:\033[37m \033[96mRSDT located at %p\033[37m\n", rsdt);

        /*if (*(uint32_t *)rsdt->signature != 'TDSR') {
            printk("\033[33macpi:\033[37m \033[91mRSDT has invalid signature\033[37m\n");
            return -1;
        }*/

       __mmap(0x0000e000, 0x70000000, 0x70000000, PAGE_READ_WRITE | PAGE_USER);
    }

    /*STD_HEADER *sdt;
    uint32_t entry_length;

    if (sdp->revision) {
        printk("\033[33macpi:\033[37m ACPI version 2.0-6.1 => using XSDT\n");

        if (__xaddr2addr(sdp->xsdt_base, &sdt, "\033[33macpi:\033[37m \033[91mXSDT address exceeds 32 bits\033[37m\n")) return -1;

        entry_length = 8;
    } else {
        printk("\033[33macpi:\033[37m \033[96mACPI version 1.0 => using RSDT\033[37m\n");

        sdt = (STD_HEADER *)sdp->rsdt_base;
        entry_length = 4;
    }*/

    /*uint32_t entries = (sdt->length - sizeof(STD_HEADER)) / entry_length;
    uint

    for (unsigned int i = 0; i < entries; ++i) {

    }*/
    
    return 0;
}