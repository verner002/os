/**
 * @file kdev.c
 * @author verner002
 * @date 02/09/2025
*/

#include "kernel/kdev.h"
#include "kstdlib/string.h"

uint16_t __dev_name_to_kdev(char const *dev_name) {
    static char *dev_names[] = {
        "hda",
        "hdb",
        "fd0"
    };

    static uint16_t dev_nums[] = {
        0x0300,
        0x0301,
        0x0200
    };

    uint32_t devs_count = sizeof(dev_names) / sizeof(char *);

    for (uint32_t i = 0; i < devs_count; ++i)
        if (!strcmp(dev_names[i], dev_name))
            return dev_nums[i];

    return NO_DEV;
}