/**
 * @file dev.c
 * @author verner002
 * @date 14/09/2025
*/

#include "hal/dev.h"
#include "kernel/heap.h"

atomic_t __dev_id;

struct __dev *__register_dev() {
    
}

struct __dev *__add_device(__kdev_t kdev, char const *name, struct __dev *parent, struct __dev_type *dev_type) {
    struct __dev *dev = (struct __dev *)kmalloc(sizeof(struct __dev));

    if (!dev)
        return NULL;

    dev->d_kdev = kdev;
    //dev->d_kobj = ; // RFC: who should be responsible for kobj allocation?
    dev->d_name = name;
    dev->d_parent = parent;
    dev->d_type = dev_type;

    dev->d_id = __dev_id;
    atomic_inc(__dev_id);
    return dev;
}