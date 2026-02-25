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
    ENOENT = 2,
    EIO = 5,
    ENOMEM = 12,
    EBUSY = 16,
    ENODEV = 19,
    ENOTDIR = 20,
    EAGAIN = 35,
    ETIMEDOUT = 110
};

/**
 * Global Variables
*/

extern int errno;