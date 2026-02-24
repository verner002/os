/**
 * @file kdev.c
 * @author verner002
 * @date 02/09/2025
*/

#include "null.h"
#include "macros.h"
#include "kernel/kdev.h"
#include "mm/heap.h"
#include "kstdlib/string.h"

//struct __kdev *kdev_top;

uint16_t __dev_name_to_kdev(char const *dev_name) {
    static char *dev_names[] = {
        "fd0",
        "hda",
        "hdb",
        "hdc",
        "hdd"
    };

    static uint16_t dev_nums[] = {
        0x0200,
        0x0300,
        0x0301,
        0x0302,
        0x0303
    };

    uint32_t devs_count = sizeof(dev_names) / sizeof(char *);

    for (uint32_t i = 0; i < devs_count; ++i)
        if (!strcmp(dev_names[i], dev_name))
            return dev_nums[i];

    return NO_DEV;
}

/*int32_t __add_kdev(__kdev_t k_kdev, struct super_block *super) {
    struct __kdev *kdev = (struct __kdev *)kmalloc(sizeof(struct __kdev));

    if (!kdev)
        return -1;

    kdev->k_dev = k_kdev;
    kdev->k_super = super;

    struct __kdev *tail;

    if (kdev_top) {
        struct __kdev *temp = kdev_top;

        do {
            tail = temp;

            if (kdev_top->k_dev == k_kdev) {
                kfree(kdev);
                return 0; // already in list
            }

            temp = temp->k_next;
        } while (likely(temp));

        // add device
        tail->k_next = kdev;
    } else
        tail = containerof(kdev_top, struct __kdev, k_next);

    tail->k_next = kdev;
    return 0;
}*/