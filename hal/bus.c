/**
 * @file bus.c
 * @author verner002
 * @date 15/09/2025
*/

#include "types.h"
#include "macros.h"
#include "hal/bus.h"
#include "mm/heap.h"

uint32_t buses_cnt = 0;
struct __bus *buses[16];

int32_t __init_buses(void) {
    return 0;
}

struct __bus *__register_bus(char const *name, struct __driver const *driver) {
    printk("bus: registering %s bus\n", name);

    if (buses_cnt >= sizeofarray(buses))
        return NULL;

    struct __bus *bus = (struct __bus *)kmalloc(sizeof(struct __bus));

    if (!bus)
        return NULL;

    struct __bus_type *bus_type = (struct __bus_type *)kmalloc(sizeof(struct __bus));

    if (!bus_type) {
        kfree(bus);
        return NULL;
    }

    bus->b_name = name;
    bus->b_type = bus_type;
    bus->b_driver = driver;
    return bus;
}