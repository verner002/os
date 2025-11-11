/**
 * @file bus.c
 * @author verner002
 * @date 15/09/2025
*/

#include "types.h"
#include "macros.h"
#include "hal/bus.h"
#include "hal/vfs.h"
#include "kernel/kobj.h"
#include "mm/heap.h"

uint32_t buses_cnt = 0;
struct __bus *buses[16];

struct __kobj_type *bus_ktype = NULL;

static void __bus_kobj_release(struct __kobj *kobj) {
    
}

static uint32_t __bus_kobj_read(struct __kobj *kobj, struct __sysfs_attrib *attrib, char *buffer) {

}

static void __bus_kobj_write(struct __kobj *kobj, struct __sysfs_attrib *attrib, char const *buffer, uint32_t size) {

}

int32_t __init_buses(void) {
    struct __kobj_type *bus_type = (struct __kobj_type *)kmalloc(sizeof(struct __kobj_type));

    if (!bus_type)
        return -1;

    bus_type->release = &__bus_kobj_release;
    bus_type->k_ops = (struct __sysfs_ops){
        .read = &__bus_kobj_read,
        .write = &__bus_kobj_write
    };
    bus_type->k_attribs = NULL;

    bus_ktype = bus_type;
    return 0;
}

struct __bus *__register_bus(char const *name, struct __driver const *driver) {
    printk("bus: registering %s bus\n", name);

    if (buses_cnt >= sizeofarray(buses))
        return NULL;

    struct __kobj *bus_kobj = (struct __kobj *)kmalloc(sizeof(struct __kobj));

    if (!bus_kobj)
        return NULL;

    __kobj_init(bus_kobj, bus_ktype);
    __kobj_rename(bus_kobj, name);
    //__kobj_add(bus_kobj, sysfs);
    //__kobj_put(bus); -- used by caller (caller should call __kobj_put)

    struct __bus *bus = (struct __bus *)kmalloc(sizeof(struct __bus));

    if (!bus) {
        __kobj_put(bus_kobj);
        return NULL;
    }

    struct __bus_type *bus_type = (struct __bus_type *)kmalloc(sizeof(struct __bus));

    if (!bus_type) {
        __kobj_put(bus_kobj);
        kfree(bus);
        return NULL;
    }

    bus->b_kobj = bus_kobj;
    bus->b_name = name;
    bus->b_type = bus_type;
    bus->b_driver = driver;
    return bus;
}