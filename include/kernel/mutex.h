/**
 * @file mutex.h
 * @author verner002
 * @date 21/02/2026
*/

#pragma once

#include "types.h"
#include "bool.h"
#include "kernel/atomic.h"

typedef struct {
    atomic_t locked;
} mutex_t;

bool __test_set(bool *lock);
void __unlock(bool *lock);
void __mutex_lock(bool *lock);
void __mutex_unlock(bool *lock);

void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);