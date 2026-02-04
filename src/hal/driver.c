/**
 * @file driver.c
 * @author verner002
 * @date 19/09/2025
*/

#include "hal/driver.h"
#include "kernel/sysfs.h"
#include "mm/heap.h"

struct __driver *drivers[256];

static struct __kobj *local_group; // local group (driver)
static uint32_t drivers_cnt = 0;

static void __group_release(struct __kobj *group) {
    // TODO: kill all drivers
    group->k_type->release(group);
}

int32_t __init_drivers(void) {
    struct __kobj *group = (struct __kobj *)kmalloc(sizeof(struct __kobj));

    if (!group)
        return -1;

    struct __kobj_type *group_type = (struct __kobj_type *)kmalloc(sizeof(struct __kobj_type));

    if (!group_type) {
        kfree(group);
        return -1;
    }

    struct __dentry *group_dentry = (struct __dentry *)kmalloc(sizeof(struct __dentry));

    if (!group_dentry) {
        kfree(group_type);
        kfree(group);
        return -1;
    }

    __dentry_init(group_dentry);
    group_dentry->name = "driver";
    __dentry_add(group_dentry, sysfs->k_dentry);

    group_type->k_attribs = NULL; // no files in /sys
    group_type->release = &__group_release;
    group_type->k_ops = (struct __sysfs_ops){
        .read = NULL,
        .write = NULL
    };

    __kobj_init(group, group_type);
    __kobj_rename(group, "driver");
    group->k_dentry = group_dentry;
    //__kobj_add(group, sysfs);

    local_group = group;

    // no driver loaded
    for (uint32_t i = 0; i < 256; ++i)
        drivers[i] = NULL;

    return 0;
}

struct __driver *__register_driver(char const *name, uint8_t major, struct __kobj_type *ktype) {
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
}

struct __driver *__driver_lookup(uint8_t major) {
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