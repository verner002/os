/**
 * @file mutex.c
 * @author verner002
 * @date 21/02/2026
*/

#include "kernel/mutex.h"
#include "kernel/task.h"

/**
 * __mutex_lock
*/

void __mutex_lock(bool *lock) {
    while (__test_set(lock))
        __yield();
}

/**
 * __mutex_unlock
*/

void __mutex_unlock(bool *lock) {
    __unlock(lock);
}

void mutex_lock(mutex_t *mutex) {
    while (atomic_xchg(&mutex->locked, 1))
        __yield();
}

void mutex_unlock(mutex_t *mutex) {
    mutex->locked = 0;
}