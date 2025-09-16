/**
 * @file kobj.c
 * @author verner002
 * @date 15/09/2025
*/

#include "types.h"
#include "macros.h"
#include "kernel/kobj.h"
#include "kernel/heap.h"
#include "kstdlib/string.h"

void __kobj_init(struct __kobj *kobj, struct __kobj_type *kobj_type) {
    if (!kobj || !kobj_type)
        return;

    kobj->k_name = NULL;
    *kobj->k_sname = '\0';
    kobj->k_refs = 1;
    kobj->k_parent = NULL;
    kobj->k_previous = NULL;
    kobj->k_next = NULL;
    kobj->k_type = kobj_type;
    kobj->k_dentry = NULL;
}

void __kobj_add(struct __kobj *kobj, struct __kobj *parent) {
    if (!kobj || !parent)
        return;

    atomic_inc(kobj->k_refs);
    kobj->k_parent = parent;
}

void __kobj_rename(struct __kobj *kobj, char const *name) {
    uint32_t length = strlen(name);

    if (length > 9) {
        kobj->k_name = name;
        *kobj->k_sname = '\0';
    } else {
        kobj->k_name = NULL;
        strcpy(kobj->k_sname, name);
    }
}

void __kobj_release(struct __kobj *kobj) {
    kobj->k_type->release(kobj);
}

void __kobj_put(struct __kobj *kobj) {
    atomic_dec(kobj->k_refs);

    if (unlikely(kobj->k_refs <= 0))
        __kobj_release(kobj);
}