/**
 * Mutex
 * 
 * Author: verner002
*/

/**
 * Includes
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