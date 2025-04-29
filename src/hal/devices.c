/**
 * Device Manager
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "hal/devices.h"

/**
 * Static Global Variables
*/

DEVICE_NODE computer = {
    .name = "Computer",
    .device = NULL,
    .subdevices = NULL
};

/**
 * __init_devman
*/

int32_t __init_devman(void) {
    uint32_t const capacity = 16;
    computer.subdevices_capacity = capacity;
    computer.subdevices_count = 0;
    
    DEVICE_NODE **subdevices = (DEVICE_NODE **)kmalloc(sizeof(DEVICE_NODE *) * capacity);

    if (!subdevices) {
        printk("Failed to allocate memory for subdevices\n");
        return -1;
    }

    memset(subdevices, 0, sizeof(DEVICE_NODE *) * capacity);
    computer.subdevices = subdevices;
    return 0;
}

/**
 * __add_device
*/

int32_t __add_device(DEVICE_NODE *parent, DEVICE_NODE *child) {
    uint32_t capacity = parent->subdevices_capacity;

    if (parent->subdevices_count >= capacity) { // reallocate array
        capacity = capacity * 2; // use some kind of GROWTH_FACTOR constant
        DEVICE_NODE **subdevices = (DEVICE_NODE **)krealloc(parent->subdevices, capacity);

        if (!subdevices)
            return -1;

        parent->subdevices = subdevices;
        parent->subdevices_capacity = capacity;
    }

    parent->subdevices[parent->subdevices_count++] = child;
    return 0;
}

/*DEVICE root = {
    .id = 0,
    .driver = NULL
};*/

/**
 * __init_devs
*/

/*void __init_devs(void) {
    root.devices = (DEVICE **)kmalloc(sizeof(DEVICE));

    DEVICE cpu = {
        .id = 1,
        .driver = NULL,
        .name = "Central Processing Unit", // replace with cpuid
        .status = EBUSY
    };

    root.devices[0] = &cpu;

    // init cpu
    cpu.status = 0; // __init_cpu()

    // look for device to initialize (use config file from initrd?)
    // create a node with status EBUSY
    // try to initialize it
    // change status to value in errno
    // repeate until all devices are listed
}*/