/**
 * @file kdev.c
 * @author verner002
 * @date 02/09/2025
*/

#include <stddef.h>
#include <string.h>

#include <kernel/kdev.h>

/**
 * ATTENTION: the list must be sorted by kdev
*/

static struct kdev_name {
    kdev_t kdev;
    char const *name;
} kdev_names[] = {
    { MAJMIN(FLOPPY_MAJOR, 0),      "fd0" },
    { MAJMIN(HARDDISK_MAJOR, 0),    "hda" },
    { MAJMIN(HARDDISK_MAJOR, 1),    "hdb" },
    { MAJMIN(HARDDISK_MAJOR, 2),    "hdc" },
    { MAJMIN(HARDDISK_MAJOR, 3),    "hdd" }
};

/**
 * name2kdev 
*/

kdev_t name2kdev(char const *name) {
    for (uint32_t i = 0; i < (sizeof(kdev_names) / sizeof(struct kdev_name)); ++i) {
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
    int right = (sizeof(kdev_names) / sizeof(struct kdev_name)) - 1;

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