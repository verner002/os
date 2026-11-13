/**
 * @file tty.c
 * @author verner002
 * @date 02/10/2025
 * 
 * @note upper-half interface for tty devices
*/

#include <stddef.h>
#include <limits.h>
#include <errno.h>

#include <kernel/tty.h>

//#define TTY_MAJOR       4

struct termio tty_table[] = {
    
};

int tty_getc(struct inode *inode) {
    struct termio *tty = tty_table + MINOR(inode->kdev);

    wait(&(tty->isem));
    char c = *tty->ihead++;

    if (tty->ihead >= tty->ibuf + IBUFLEN)
        tty->ihead = tty->ibuf;

    if (tty->eof && c == tty->eofc)
        return EOF;

    return c;
}

int tty_putc(struct inode *inode, int c) {
    struct termio *tty = tty_table + MINOR(inode->kdev);

    wait(&(tty->osem));
    *tty->otail++ = c;

    if (tty->otail >= tty->obuf + OBUFLEN)
        tty->otail = tty->obuf;

    tty->kickout();
    return 0;
}

int tty_read(struct inode *inode, struct file *file, void *buffer, size_t count) {
    if (count > SSIZE_MAX) {
        errno = EINVAL;
        return -1;
    }
    
    char *ptr = buffer;
    int read;

    for (read = 0; read < count; ++read) {
        int ch = tty_getc(inode);

        if (ch == EOF)
            break;

        *ptr++ = ch;
    }

    return read;
}

int tty_write(struct inode *inode, struct file *file, void const *buffer, size_t count) {
    if (count > SSIZE_MAX) {
        errno = EINVAL;
        return -1;
    }

    char *ptr = buffer;
    int written;

    for (written = 0; written < count; ++written)
        tty_putc(inode, *ptr++);

    return written;
}