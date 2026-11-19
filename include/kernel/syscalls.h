/**
 * @file syscalls.h
 * @author verner002
 * @date 24/09/2026
*/

#pragma once

#define NR_fork         0
#define NR_open         1
#define NR_read         2
#define NR_write        3
#define NR_close        4
#define NR_pause        5

static inline int fork(void) {
    int result;
    
    asm volatile (
        "int 0x80"
        : "=a" (result)
        : "0" (NR_fork)
        :
    );

    return result;
}

static inline int open(char const *path, int flags, int mode) {
    int result;

    asm volatile (
        "int 0x80"
        : "=a" (result)
        : "0" (NR_open), "b" (path), "c" (flags), "d" (mode)
        :
    );

    return result;
}

static inline int write(int fd, char const *buffer, size_t count) {
    int result;

    asm volatile (
        "int 0x80"
        : "=a" (result)
        : "0" (NR_write), "b" (fd), "c" (buffer), "d" (count)
        :
    );

    return result;
}

static inline int close(int fd) {
    int result;

    asm volatile (
        "int 0x80"
        : "=a" (result)
        : "0" (NR_close), "b" (fd)
        :
    );

    return result;
}

static inline int pause(void) {
    int result;

    asm volatile (
        "int 0x80"
        : "=a" (result)
        : "0" (NR_pause)
        :
    );

    return result;
}