/**
 * @file acpi.c
 * @author verner002
 * @date 17/08/2025
*/

#include "drivers/acpi.h"

uint32_t acpi_version;
bool legacy_ps2c_installed;
bool legacy_pics_installed;

/**
 * __find_rsdp
*/

RSDP *__find_rsdp(void *start, uint32_t size) {
    for (uint32_t i = 0; i < size; i += 16) { // 16-byte alignment
        void *ptr = start + i;

        if (!strncmp((char const *)ptr, "RSD PTR ", 8))
            return (RSDP *)ptr;
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

static char *__apic_etype2str(uint8_t type) {
    switch (type) {
        case 0:
            return "CPU LAPIC"; // local apic

        case 1:
            return "I/O APIC";

        case 2:
            return "I/O APIC ISO"; // interrupt source override

        case 3:
            return "I/O NMIS"; // non-maskable interrupt source

        case 4:
            return "LAPIC NMI"; // local api non-maskable interrupts
        
        case 5:
            return "LAPIC AO"; // local apic address override

        case 9:
            return "CPU LAPIC 2x"; // local apic
    }

    return "Unknown";
}

/**
 * __parse_apic_tbl
*/

static void __parse_apic_table(SDT_HEADER *hdr) {
    APIC *apic = (APIC *)hdr;

    uint32_t read = 0;

    printk("\033[33macpi-apic:\033[37m APIC table length %u\n", apic->h.length);

    uint32_t data_len = apic->h.length - sizeof(APIC);

    printk("\033[33macpi-apic:\033[37m APIC data length %u\n", data_len);

    printk("\033[33macpi-apic:\033[37m Local APIC address %p\n", apic->local_apic_addr);
    printk("\033[33macpi-apic:\033[37m Local APIC flags %p\n", apic->flags);

    if (apic->flags & 0x00000001)
        printk("\033[33macpi-apic:\033[37m \033[96mDual legacy PICs installed\033[37m\n");
    
    uint8_t *data = (uint8_t *)apic + sizeof(APIC);

    while (read < data_len) {
        APIC_HEADER *apic_h = (APIC_HEADER *)data;
        uint32_t detected_length = apic_h->length;
        uint32_t expected_length;

        switch (apic_h->type) {
            case APIC_TYPE_CPU_LAPIC:
                expected_length = sizeof(APIC_CPU_LAPIC);
                
                printk("Found CPU LAPIC entry\n");
                break;

            case APIC_TYPE_IO_APIC:
                expected_length = sizeof(APIC_IO_APIC);

                printk("Found I/O APIC entry\n");
                break;

            /*case APIC_TYPE_IO_APIC_ISO:
                break;

            case APIC_TYPE_IO_NMIS:
                break;

            case APIC_TYPE_LAPIC_AO:
                break;

            case APIC_TYPE_CPU_DLAPIC:
                break;*/

            default:
                expected_length = 0;

                printk("Found unknown entry (%u)\n", apic_h->type);
                break;
        }
        
        if (expected_length > 0 && detected_length != expected_length) {
            printk("\033[91mThe entry may be corrupted.\033[37m\n");
            printk("\033[91mExpected %u byte(s), got %u byte(s)\033[37m\n", expected_length, detected_length);
        }

        data += detected_length;
        read += detected_length;
    }

    // TODO: calculate checksum
}

static void __parse_fadt_table(SDT_HEADER *hdr) {
    // check ia pc boot architecture flags (0 - no ps/2 controller)
    FADT *fadt = (FADT *)hdr;

    printk("\033[33macpi-fadt:\033[37m Calculating FADT checksum... ");

    if (__checksum((uint8_t *)fadt, fadt->h.length)) {
        printf("Invalid\n");
        return;
    }

    printf("Valid\n");

    if (!acpi_version || (acpi_version == 2 && fadt->ia_boot_arch_flags & 0x0002))
        printk("\033[33macpi-fadt:\033[37m Legacy PS/2 controller installed\n");
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

    acpi_version = rsdp->revision;

    if (acpi_version == 2) {
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

        // we need this if-block here because the remaining
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

            for (uint32_t j = 0; j < 4; ++j)
                putchar(sdt_h->signature[j]);

            putchar('\n');

            if (*(uint32_t *)sdt_h->signature == (uint32_t)'CIPA')
                __parse_apic_table(sdt_h);
            else if (*(uint32_t *)sdt_h->signature == (uint32_t)'PCAF')
                __parse_fadt_table(sdt_h);
            else
                printk("\033[33macpi:\033[37m Parser not implemented\n");
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