/**
 * @file tty.c
 * @author verner002
 * @date 02/10/2025
*/

#pragma once

#include <kernel/process.h>

#define IBUFLEN         16
#define OBUFLEN         16

#define EOF             -1

struct termio {
    char *ihead;
    char *itail;
    char ibuf[IBUFLEN];
    struct semaphore isem;
    char *ohead;
    char *otail;
    char obuf[OBUFLEN];
    struct semaphore osem;
    int eof;
    char eofc;
    void (*kickout)(void);
};

extern struct termio tty_table[];