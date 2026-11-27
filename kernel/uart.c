/**
 * @file uart.c
 * @author verner002
 * @date 03/11/2026
*/

#include <stddef.h>
#include <errno.h>

#include <kernel/types.h>
#include <kernel/io.h>
#include <kernel/idt.h>
#include <kernel/irqs.h>
#include <kernel/tty.h>

#define COM1            0x3f8   // com1 port number
#define COM2            0x2f8   // com2 port number
#define COM3            0x3e8   // com3 port number
#define COM4            0x2e8   // com4 port number

#define RBR             0       // dlab=0, read
#define THR             0       // dlab=0, write
#define IER             1       // dlab=0, read/write
#define DLR             0       // dlab=1, read/write
#define DHR             1       // dlab=1, read/write
#define IIR             2       // read
#define FCR             2       // write
#define LCR             3       // read/write
#define MCR             4       // read/write
#define LSR             5       // read
#define MSR             6       // read
#define SCR             7       // read/write

// interrupt identification register
#define FIFO_ENABLED    0x80
#define FIFO_USABLE     0x40
#define INT_PENDING     0x01

// line control bits
#define DLAB_ACCESS     0x80
#define BREAK_ENABLE    0x40

#define PARITY_NONE     (0 << 3)
#define PARITY_ODD      (1 << 3)
#define PARITY_EVEN     (3 << 3)
#define PARITY_MARK     (5 << 3)
#define PARITY_SPACE    (7 << 3)

#define STOP1           (0 << 2)
#define STOP2           (1 << 2)

#define CHAR5           (0 << 0)
#define CHAR6           (1 << 0)
#define CHAR7           (2 << 0)
#define CHAR8           (3 << 0)

// interrupt control bits
#define MODEM_STATUS    8
#define RX_LINE_STATUS  4
#define TX_HOLD_EMPTY   2
#define DATA_AVAILABLE  1

// fifo control bits
#define TRIGGER1        (0 << 6)
#define TRIGGER4        (1 << 6)
#define TRIGGER8        (2 << 6)
#define TRIGGER14       (3 << 6)

#define DMA_ENABLE      8
#define CLEAR_TX_FIFO   4
#define CLEAR_RX_FIFO   2
#define FIFO_ENABLE     1

// modem control register
#define DATA_TERM_RDY   1   // terminal is prepared for communication
#define REQ_TO_SEND     2   // terminal is ready for data
#define OUT1            4   // not used on pc
#define OUT2            8   // enable irq
#define LOOPBACK        16  // enable loopback

// line status register
#define DATA_RDY        0x01
#define THR_EMPTY       0x20

// interrupt enable register
#define RX_IRQ          1
#define TX_IRQ          2

#define FIFO_SIZE       16

// internal clock tick rate
#define CLOCK           115200u

// supported rates
#define B9600           (CLOCK / 9600u)

static struct tty *com1_tty;

void uart_kickout(struct tty *tty) {
    if (!(inb(COM1 + LSR) & THR_EMPTY))
        return;

    // enforce new thre interrupt
    uint8_t ier = inb(COM1 + IER);
    outb(COM1 + IER, ier & ~TX_IRQ);
    outb(COM1 + IER, ier | TX_IRQ);
}

__attribute__((interrupt)) static void com1_handler(struct interrupt_frame *) {    
    acknowledge_irq(4);

    // is it our problem?
    if (inb(COM1 + IIR) & INT_PENDING)
        return;

    uint8_t ls;
    
    while ((ls = inb(COM1 + LSR)) & DATA_RDY) {
        char c = inb(COM1 + RBR);

        int count;
        sem_read(&(com1_tty->rx_sem), &count);

        if (count < RX_BUF_LEN) {
            *com1_tty->rx_tail++ = c;

            if (com1_tty->rx_tail >= com1_tty->rx_buf + RX_BUF_LEN)
                com1_tty->rx_tail = com1_tty->rx_buf;

            sem_signal(&(com1_tty->rx_sem));
        }
    }

    if (ls & THR_EMPTY) {
        for (size_t i = 0; i < FIFO_SIZE; ++i) {
            // fill thr
            int count;
            sem_read(&(com1_tty->tx_sem), &count);

            if (count >= TX_BUF_LEN)
                break;

            char c = *com1_tty->tx_head++;

            if (com1_tty->tx_head >= (com1_tty->tx_buf + TX_BUF_LEN))
                com1_tty->tx_head = com1_tty->tx_buf;

            outb(COM1 + THR, c);
            sem_signal(&(com1_tty->tx_sem));
        }
    }
}

static struct tty_ops const uart_ops = {
    .kickout = &uart_kickout
};

int uart_init(void) {
    outb(COM1 + LCR, DLAB_ACCESS);
    outb(COM1 + DLR, (uint8_t)(B9600 & 0xff));
    outb(COM1 + DHR, (uint8_t)((B9600 >> 8) & 0xff));
    outb(COM1 + LCR, PARITY_NONE | STOP1 | CHAR8);
    
    // disable com1 irqs (shared with com3)
    outb(COM1 + IER, 0);

    // ----------------------------------------
    // disable com2-4, we won't use them now
    // disable com2 irqs (shared with com4)
    outb(COM2 + LCR, 0);
    outb(COM2 + IER, 0);

    // disable com3 irqs (shared with com1)
    outb(COM3 + LCR, 0);
    outb(COM3 + IER, 0);

    // disable com2 irqs (shared with com2)
    outb(COM4 + LCR, 0);
    outb(COM4 + IER, 0);
    // ----------------------------------------

    // enable fifos
    outb(COM1 + FCR, FIFO_ENABLE);

    int iir = inb(COM1 + IIR);
    
    if ((iir & (FIFO_ENABLED | FIFO_USABLE)) == (FIFO_ENABLED | FIFO_USABLE)) {
        // 16550a, we can safely use fifos
        outb(COM1 + FCR, TRIGGER14 | CLEAR_TX_FIFO | CLEAR_RX_FIFO | FIFO_ENABLE);
    } else {
        // older models, disable fifos
        outb(COM1 + FCR, 0);
    }

    // enable loopback, out2 (enable irq), rts and dtr
    outb(COM1 + MCR, LOOPBACK | OUT2 | DATA_TERM_RDY | REQ_TO_SEND);

    // check if port is faulty
    outb(COM1 + THR, 0xae);

    int timeout = 1000;

    while (!(inb(COM1 + LSR) & DATA_RDY))
        if (!--timeout)
            return -EBUSY;

    if (inb(COM1 + RBR) != 0xae)
        return -ENODEV;
    
    // disable loopback
    outb(COM1 + MCR, OUT2 | DATA_TERM_RDY | REQ_TO_SEND);

    // clear events bits
    inb(COM1 + RBR);
    inb(COM1 + LSR);
    inb(COM1 + MSR);
    inb(COM1 + IIR);

    struct tty *tty = tty_register(&uart_ops, NULL, "ttyS0");

    if (!tty)
        return -ENODEV;

    com1_tty = tty;

    // com1 handler
    install_interrupt_gate(0x24, &com1_handler);

    // enable irq 4
    enable_irq(4);

    // enable rx interrupt
    outb(COM1 + IER, RX_IRQ);

    return 0;
}