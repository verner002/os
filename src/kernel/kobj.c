/**
 * @file kobj.c
 * @author verner002
 * @date 15/09/2025
*/

#include "kernel/kobj.h"

struct __kobj *__kobj_init() {
    // initialize kobj
}

void __kobj_release(struct __kobj *kobj) {
    // free kobj
}

void __kobj_put(struct __kobj *kobj) {
    if (--kobj->k_refs <= 0)
        __kobj_release(kobj);
}