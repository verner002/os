/**
 * @file tty.c
 * @author verner002
 * @date 02/10/2025
*/

#pragma once

#include <kernel/semaphore.h>

#define RX_BUF_LEN      16
#define TX_BUF_LEN      16

#define EOF             -1

#define TTY_GETMODE    0x5501
#define TTY_SETMODE    0x5502

enum uart_mode {
    TTY_MODE_POLL,
    TTY_MODE_IRQ
};

// forward declarions
struct tty;

struct termio {
    int eof;
    char eofc;
};

struct tty_ops {
    void (*kickout)(struct tty *tty);
};

struct tty {
    int free;
    int minor;
    struct device *dev;

    struct termio termio;

    char *rx_head;
    char *rx_tail;
    char rx_buf[RX_BUF_LEN];
    sem_t rx_sem;

    char *tx_head;
    char *tx_tail;
    char tx_buf[TX_BUF_LEN];
    sem_t tx_sem;

    struct tty_ops const *ops;
    void *private_data;
};

int tty_init(void);
struct tty *tty_register(struct tty_ops const *ops, void *private_data, char const *name);