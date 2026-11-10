/**
 * @file driver.c
 * @author verner002
 * @date 19/09/2025
*/

#include "macros.h"
#include "hal/driver.h"
#include "kernel/sysfs.h"
#include "mm/heap.h"

struct __driver *drivers[256];

int drivers_init(void) {
    // no driver loaded
    for (uint32_t i = 0; i < sizeofarray(drivers); ++i)
        drivers[i] = NULL;

    return 0;
}

/*struct __driver *__register_driver(char const *name, uint8_t major, struct __kobj_type *ktype) {
    struct __driver *driver = (struct __driver *)kmalloc(sizeof(struct __driver));

    if (!driver)
        return NULL;

    struct __kobj *kobj = (struct __kobj *)kmalloc(sizeof(struct __kobj));

    if (!kobj) {
        kfree(driver);
        return NULL;
    }

    struct __dentry *dentry = (struct __dentry *)kmalloc(sizeof(struct __dentry));

    if (!dentry) {
        kfree(kobj);
        kfree(driver);
        return NULL;
    }

    __dentry_init(dentry);
    dentry->name = name;
    __dentry_add(dentry, local_group->k_dentry);

    __kobj_init(kobj, ktype);
    __kobj_rename(kobj, name);
    __kobj_add(kobj, sysfs);

    //__kobj_put(bus); -- used by caller (caller should call __kobj_put)

    driver->kobj = kobj;
    driver->release = NULL;
    drivers[major] = driver;
    return driver;
}*/

int register_blk_driver(uint8_t major, int (*read)(uint8_t minor, uint32_t offset, uint32_t count, char *buffer)) {
    struct __block_dev_driver *driver = (struct __block_dev_driver *)kmalloc(sizeof(struct __block_dev_driver));

    if (!driver)
        return -1;

    driver->h.type = NULL;
    driver->h.release = NULL;
    driver->read = read;
    driver->write = NULL;
    driver->ioctl = NULL;

    drivers[major] = (struct __driver *)driver;
    return 0;
}

struct __driver *driver_lookup(uint8_t major) {
    return drivers[major];
}

/**
 * __link_symbol
*/

/*uint32_t __link_symbol(DRIVER *driver, char const *name) {
    uint32_t symbols_count = driver->symbols_count;
    SYMBOL *symbols = driver->symbols;
    
    for (uint32_t i = 0; i < symbols_count; ++i) {
        SYMBOL *symbol = &symbols[i];

        if (!strcmp((char const *)symbol->name, name))
            return symbol->address;
    }

    return 0;
}*/