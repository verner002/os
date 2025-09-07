/**
 * Mutex
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "types.h"
#include "bool.h"

/**
 * Declarations
*/

bool __test_set(bool *lock);
void __mutex_lock(bool *lock);
void __mutex_unlock(bool *lock);