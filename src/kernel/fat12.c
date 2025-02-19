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

static uint8_t *fat = (uint8_t *)0x00010000;
static FAT12_RECORD *root_dir = (FAT12_RECORD *)0x00020000;

/**
 * __fat12_read_fat
*/

uint32_t __fat12_read_fat(void) {
    return __fdc_read_sectors(1, 9, (uint32_t)fat);
}

/**
 * __fat12_read_root_dir
*/

uint32_t __fat12_read_root_dir(void) {
    return __fdc_read_sectors(9 * 2 + 1, 224 * 32 / 512, (uint32_t)root_dir);
}

/**
 * __fat12_load_file
*/

uint32_t __fat12_load_file(char const *filename, uint32_t buffer) {
    for (uint32_t i = 0; i < 224; ++i) {
        FAT12_RECORD *record = &root_dir[i];

        if (!strncmp(record->filename, filename, 11)) {
            printk("Found %s\n", filename);
            uint32_t cluster = record->first_cluster;

            do {
                uint32_t lba = ((cluster - 2) * 1) + 1 + (2 * 9) + (224 * 32 / 512);
                __fdc_read_sectors(lba, 1, buffer);
                uint32_t next_cluster = *(uint16_t *)(fat + cluster + cluster / 2);

                if (cluster & 1) next_cluster >>= 4;

                next_cluster &= 0x00000fff;
                cluster = next_cluster;
                buffer += 512;
                printk("%u\n", cluster);
            } while (cluster < 0x00000ff8);

            return 0;
        }
    }

    return -1;
}