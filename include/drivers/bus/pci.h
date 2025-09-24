/**
 * @file pci.c
 * @author verner002
 * @date 04/09/2025
*/

#pragma once

#include "types.h"

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

struct __pci_h_device {
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

int32_t __init_pci(void);
uint32_t __pci_fix_bar(uint32_t bar);