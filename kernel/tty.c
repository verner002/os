/**
 * @file tty.c
 * @author verner002
 * @date 02/10/2025
 * 
 * @note upper-half interface for tty devices
*/

#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#include <kernel/tty.h>
#include <kernel/fcntl.h>
#include <kernel/inode.h>
#include <kernel/dcache.h>
#include <kernel/device.h>

#define MAX_TTYS        4 // tities haha
#define TTY_MAJOR       4

static struct cdev tty_cdev;

static struct tty tty_table[MAX_TTYS];

int tty_ioctl(/*struct device *dev, */unsigned short cmd, void *data) {
    // TODO: implement polling mode, if not possible
    //  read/write should return EWOULDBLOCK/EAGAIN

    switch (cmd) {
        case TTY_GETMODE:
            break;

        case TTY_SETMODE:
            break;
    }

   return -ENOSYS;
}

int tty_getc(struct inode *inode) {
    struct tty *tty = tty_table + MINOR(inode->rdev);

    sem_wait(&(tty->rx_sem));
    char c = *tty->rx_head++;

    if (tty->rx_head >= tty->rx_buf + RX_BUF_LEN)
        tty->rx_head = tty->rx_buf;

    if (tty->termio.eof && c == tty->termio.eofc)
        return EOF;

    return c;
}

int tty_putc(struct inode *inode, int c) {
    struct tty *tty = tty_table + MINOR(inode->rdev);

    sem_wait(&(tty->tx_sem));
    *tty->tx_tail++ = c;

    if (tty->tx_tail >= tty->tx_buf + TX_BUF_LEN)
        tty->tx_tail = tty->tx_buf;

    if (tty->ops && tty->ops->kickout)
        tty->ops->kickout(tty);
    
    return 0;
}

static ssize_t tty_open(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t tty_read(struct file *file, char *buffer, size_t count) {
    if (count > SSIZE_MAX) {
        errno = EINVAL;
        return -1;
    }
    
    char *ptr = buffer;
    ssize_t read;
    struct inode *inode = file->dentry->inode;

    for (read = 0; read < (ssize_t)count; ++read) {
        int ch = tty_getc(inode);

        if (ch == EOF)
            break;

        *ptr++ = ch;
    }

    return read;
}

static ssize_t tty_write(struct file *file, char const *buffer, size_t count) {    
    if (count > SSIZE_MAX) {
        errno = EINVAL;
        return -1;
    }

    char const *ptr = buffer;
    ssize_t written;
    struct inode *inode = file->dentry->inode;

    for (written = 0; written < (ssize_t)count; ++written)
        tty_putc(inode, *ptr++);

    return written;
}

struct tty *tty_register(struct tty_ops const *ops, void *private_data, char const *name) {
    for (size_t i = 0; i < MAX_TTYS; ++i) {
        struct tty *tty = tty_table + i;

        if (tty->free) {
            struct device *dev = device_create(MAJMIN(TTY_MAJOR, i), name);

            if (!dev)
                return NULL;

            tty->free = 0;
            tty->minor = i;
            tty->dev = dev;
            
            tty->rx_head = tty->rx_buf;
            tty->rx_tail = tty->rx_buf;
            sem_init(&(tty->rx_sem), 0);

            tty->tx_head = tty->tx_buf;
            tty->tx_tail = tty->tx_buf;
            sem_init(&(tty->tx_sem), TX_BUF_LEN);

            tty->ops = ops;
            tty->private_data = private_data;

            return tty;
        }
    }

    return NULL;
}

static struct file_ops const tty_fops = {
    .open = &tty_open,
    .read = &tty_read,
    .write = &tty_write
};

int tty_init(void) {
    int err;

    cdev_init(&tty_cdev, &tty_fops);

    err = cdev_add(&tty_cdev, MAJMIN(TTY_MAJOR, 0), MAX_TTYS);

    if (err < 0)
        return err;

    for (size_t i = 0; i < MAX_TTYS; ++i) {
        struct tty *tty = tty_table + i;

        memset(tty, 0, sizeof(struct tty));
        tty->free = 1;
    }

    return 0;
}