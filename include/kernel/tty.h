/**
 * @file tty.c
 * @author verner002
 * @date 02/10/2025
*/

#pragma once

#include <kernel/semaphore.h>

#define IBUFLEN         16
#define OBUFLEN         16

#define EOF             -1

#define TTY_GETMODE    0x5501
#define TTY_SETMODE    0x5502

enum uart_mode {
    UART_MODE_POLL,
    UART_MODE_IRQ
};

struct termio {
    char *ihead;
    char *itail;
    char ibuf[IBUFLEN];
    sem_t isem;
    char *ohead;
    char *otail;
    char obuf[OBUFLEN];
    sem_t osem;
    int eof;
    char eofc;
    void (*kickout)(void);
};

extern struct termio tty_table[];