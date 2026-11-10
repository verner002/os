/**
 * @file fcntl.h
 * @author verner002
 * @date 15/10/2026
*/

#pragma once

// access modes
#define O_RDONLY        0
#define O_WRONLY        1
#define O_RDWR          2

// creation flags
#define O_CREAT         0x10
#define O_EXCL          0x20    // if file already exists => EEXIST
#define O_TRUNC         0x40    // if exists, truncated to 0 bytes after open

// status flags
#define O_APPEND        0x100   // seek to end before each write
#define O_NONBLOCK      0x200   // read/write returns EAGAIN/EWOULDBLOCK instead of blocking
#define O_CLOEXEC       0x400   // closes file descriptor on exec*() calls