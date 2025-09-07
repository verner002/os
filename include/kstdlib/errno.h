/**
 * Error
 * 
 * Author: verner002
*/

#pragma once

/**
 * Enumerations
*/

enum {
    EIO = 5,
    EBUSY = 16,
    ENODEV = 19,
    EAGAIN = 35,
    ETIMEDOUT = 110
};

/**
 * Global Variables
*/

extern int errno;