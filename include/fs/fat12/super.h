/**
 * @file super.h
 * @author verner002
 * @date 04/11/2025
*/

#pragma once

#include "fs/super.h"
#include "kernel/kdev.h"

struct __attribute__((__packed__)) fat12_info {
    uint8_t     reserved1[3];
    uint8_t     oem_label[8];
    uint16_t    bytes_per_sector;
    uint8_t     sectors_per_cluster;
    uint16_t    reserved_sectors;
    uint8_t     number_of_fats;
    uint16_t    number_of_entries;
    uint16_t    sectors_total;
    uint8_t     media_type;
    uint16_t    sectors_per_fat;
    uint16_t    sectors_per_track;
    uint16_t    heads_per_cylinder;
    uint32_t    number_of_hidden_sectors;
    uint32_t    sectors_total_long;
    uint8_t     drive_number;
    uint8_t     reserved2;
    uint8_t     signature;
    uint32_t    serial_number;
    uint8_t     volume_label[11];
    uint8_t     file_system[8];
    uint8_t     reserved3[450];
};

int get_super(kdev_t kdev, struct super_block *super);