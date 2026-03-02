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

/**
 * ATTENTION: the list must be sorted by kdev
*/

static struct kdev_name {
    kdev_t kdev;
    char const *name;
} kdev_names[] = {
    { .kdev = MAJMIN(FLOPPY_MAJOR, 0), .name = "fd0" },
    { .kdev = MAJMIN(HARDDISK_MAJOR, 0), .name = "hda" },
    { .kdev = MAJMIN(HARDDISK_MAJOR, 1), .name = "hdb" },
    { .kdev = MAJMIN(HARDDISK_MAJOR, 2), .name = "hdc" },
    { .kdev = MAJMIN(HARDDISK_MAJOR, 3), .name = "hdd" }
};

/**
 * name2kdev 
*/

kdev_t name2kdev(char const *name) {
    for (uint32_t i = 0; i < sizeofarray(kdev_names); ++i) {
        struct kdev_name *kdev_name = &kdev_names[i];

        if (!strcmp(name, kdev_name->name))
            return kdev_name->kdev;
    }

    return NO_DEV;
}

/**
 * kdev2name
*/

char const *kdev2name(kdev_t kdev) {
    int left = 0;
    int right = sizeofarray(kdev_names) - 1;

    while (left <= right) {
        int middle = (left + right) / 2;
        struct kdev_name *kdev_name = &kdev_names[middle];

        if (kdev < kdev_name->kdev)
            right = middle - 1;
        else if (kdev > kdev_name->kdev)
            left = middle + 1;
        else
            return kdev_name->name;
    }

    return NULL;
}

/*int32_t __add_kdev(kdev_t k_kdev, struct super_block *super) {
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