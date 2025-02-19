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
    uint32_t xaddress_h = xaddress >> 32; // FIXME: check if compiled correctly
    
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
    for (uint32_t i = 0; i < size; i += 16) { // 16-byte alignment
        uint64_t *ptr = start + i;

        if (*ptr == ' RTP DSR') return (RSDP *)(ptr);
    }

    return NULL;
}

/**
 * __checksum
*/

static uint8_t __checksum(uint8_t *table, uint32_t length) {
    uint8_t checksum = 0;

    for (uint32_t i = 0; i < length; ++i)
        checksum += table[i];

    return checksum;
}

/**
 * __map_sdt
*/

SDT *__map_sdt(SDT *sdt) {
    
}

/**
 * __init_acpi
*/

int32_t __init_acpi(void) {
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

    if (__checksum((uint8_t *)rsdp, sizeof(RSDP))) {
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
         *  paging is enabled, we can't access retrieved addresses,
         *  check if addresses are mapped, if not use static paging
         *  table to map the memory (paging directory for kernel
         *  passed by loader)
        */
        
        SDT_HEADER *rsdt = (SDT_HEADER *)rsdp->rsdt_base;

        printk("\033[33macpi:\033[37m \033[96mRSDT located at PAS=%p\033[37m\n", rsdt);

        asm ("mov %0, cr3" : "=r" (page_directory) ::);
        page_directory &= ~31; // discards cd, wt and res

        uint32_t
            rsdt_origin = 0x90000000,
            page_rsdt = (uint32_t)rsdt & 0xfffff000,
            offset_rsdt = (uint32_t)rsdt & 0x00000fff,
            last_address = rsdt_origin,
            pages_loaded = 1;

        if (__map_page(last_address, page_rsdt, PAGE_NONE)) {
            printk("Failed to map VAS=%p to PAS=%p\n", last_address, page_rsdt);
            return -1;
        }

        // i need this if-block here because the remaining
        // part of the table (with length) may not be mapped
        if (offset_rsdt + sizeof(SDT_HEADER) > 0x00001000) {
            page_rsdt += 0x00001000;
            last_address += 0x00001000;
            ++pages_loaded;

            if (__map_page(last_address, page_rsdt, PAGE_NONE)) {
                printk("Failed to map VAS=%p to PAS=%p\n", last_address, page_rsdt);
                return -1;
            }
        }
        
        rsdt = (SDT_HEADER *)(rsdt_origin + offset_rsdt);

        // check signature (we cannot use checksum yet)
        if (*(uint32_t *)rsdt->signature != 'TDSR') {
            printk("\033[33macpi:\033[37m \033[91mRSDT has invalid signature\033[37m\n");
            return -1;
        }

        // now i can use the length so we can load the whole
        // table without worries
        uint32_t pages_to_load = (offset_rsdt + rsdt->length + 0xfff) / 0x1000 - pages_loaded;

        for (uint32_t i = 0; i < pages_to_load; ++i) { // 22 4-byte pointers
            page_rsdt += 0x1000;
            last_address += 0x1000;

            if (__map_page(last_address, page_rsdt, PAGE_NONE)) {
                printk("Failed to map VAS=%p to PAS=%p\n", last_address, page_rsdt);
                return -1;
            }
        }

        printk("\033[33macpi:\033[37m Calculating RSDT checksum... ");

        if (__checksum((uint8_t *)rsdt, rsdt->length)) {
            printf("Invalid\n");
            return -1;
        }

        printf("Valid\n");

        uint32_t *ptrs = (uint32_t *)((uint32_t)rsdt + sizeof(SDT_HEADER));
        uint32_t ptrs_count = (rsdt->length - sizeof(SDT_HEADER)) / sizeof(uint32_t);

        printk("\033[33macpi:\033[37m Found SDTs (%u):\n", ptrs_count);

        //printf("%p\n", *(uint32_t *)(((uint32_t)rsdt + sizeof(SDT_HEADER) + 4)));

        for (uint32_t i = 0; i < ptrs_count; ++i) {
            SDT_HEADER *sdt_h = (SDT_HEADER *)ptrs[i];

            uint32_t
                madt_h_page = (uint32_t)sdt_h & 0xfffff000,
                madt_h_offset = (uint32_t)sdt_h & 0x00000fff;

            if (__map_page(0xa0000000, madt_h_page, PAGE_NONE)) {
                printk("Failed to map VAS=%p to PAS=%p\n", 0xa0000000, madt_h_page);
                return -1;
            }

            if (madt_h_offset + sizeof(SDT_HEADER) > 0x00001000) {
                if (__map_page(0xa0001000, madt_h_page + 0x00001000, PAGE_NONE)) {
                    printk("Failed to map VAS=%p to PAS=%p\n", 0xa0001000, madt_h_page + 0xa0001000);
                    return -1;
                }
            }

            sdt_h = (SDT_HEADER *)(0xa0000000 + madt_h_offset);

            printf("                      - PAS=%p, VAS=%p, SIGN=", ptrs[i], sdt_h);

            for (uint32_t j = 0; j < 4; ++j) putchar(sdt_h->signature[j]);

            putchar('\n');
        }
    }

    /*SDT_HEADER *sdt;
    uint32_t entry_length;

    if (sdp->revision) {
        printk("\033[33macpi:\033[37m ACPI version 2.0-6.1 => using XSDT\n");

        if (__xaddr2addr(sdp->xsdt_base, &sdt, "\033[33macpi:\033[37m \033[91mXSDT address exceeds 32 bits\033[37m\n")) return -1;

        entry_length = 8;
    } else {
        printk("\033[33macpi:\033[37m \033[96mACPI version 1.0 => using RSDT\033[37m\n");

        sdt = (SDT_HEADER *)sdp->rsdt_base;
        entry_length = 4;
    }*/

    /*uint32_t entries = (sdt->length - sizeof(SDT_HEADER)) / entry_length;
    uint

    for (unsigned int i = 0; i < entries; ++i) {

    }*/
    
    return -1; // TODO: change latter, we're not ready to use acpi yet
}