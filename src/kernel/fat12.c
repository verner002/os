/**
 * File Allocation Table 12
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kernel/fat12.h"

/**
 * Static Global Variables
*/

static uint8_t *fat = NULL;
static FAT12_RECORD *root_dir = NULL;

/**
 * __fat12_read_fat
*/

int32_t __fat12_read_fat(void) {
    if (!fat) {
        fat = (uint8_t *)e820_rmalloc(9*512, FALSE);

        if (!fat) return -1;
    }

    return __fdc_read_sectors(1, 9, (uint32_t)fat);
}

/**
 * __fat12_read_root_dir
*/

int32_t __fat12_read_root_dir(void) {
    if (!root_dir) {
        root_dir = (FAT12_RECORD *)e820_rmalloc(224*32, FALSE);

        if (!root_dir) return -1;
    }

    return __fdc_read_sectors(9 * 2 + 1, 224 * 32 / 512, (uint32_t)root_dir);
}

/**
 * __fat12_load_file
*/

int32_t __fat12_load_file(char const *filename, uint32_t buffer) {
    for (uint32_t i = 0; i < 224; ++i) {
        FAT12_RECORD *record = &root_dir[i];

        if (!strncmp(record->filename, filename, 11)) {
            uint32_t cluster = record->first_cluster;

            do {
                uint32_t lba = ((cluster - 2) * 1) + 1 + (2 * 9) + (224 * 32 / 512);
                __fdc_read_sectors(lba, 1, buffer);
                uint32_t next_cluster = *(uint16_t *)(fat + cluster + cluster / 2);

                if (cluster & 1) next_cluster >>= 4;

                next_cluster &= 0x00000fff;
                cluster = next_cluster;
                buffer += 512;
            } while (cluster < 0x00000ff8);

            return 0;
        }
    }

    return -1;
}