/**
 * @file tty.c
 * @author verner002
 * @date 02/10/2025
*/

#include <stddef.h>

#include <kernel/tty.h>

//#define TTY_MAJOR       4

struct termio tty_table[] = {
    
};

int tty_getc(struct inode *inode, struct file *file) {
    struct termio *tty = tty_table + MINOR(inode->kdev);

    wait(&(tty->isem));
    char c = *tty->ihead++;

    if (tty->ihead >= tty->ibuf + IBUFLEN)
        tty->ihead = tty->ibuf;

    if (tty->eof && c == tty->eofc)
        return EOF;

    return c;
}

int tty_putc(struct inode *inode, struct file *file, int c) {
    struct termio *tty = tty_table + MINOR(inode->kdev);

    wait(&(tty->osem));
    *tty->otail++ = c;

    if (tty->otail >= tty->obuf + OBUFLEN)
        tty->otail = tty->obuf;

    tty->kickout();
    return 0;
}

int tty_read(struct inode *inode, struct file *file, void *buffer, size_t count) {
    char *ptr = buffer;
    int read;

    int ch = tty_getc(inode, file);

    if (ch == EOF)
        return EOF;

    for (read = 1; read < count; ++read) {
        ch = tty_getc(inode, file);

        if (ch == EOF)
            break;

        *ptr++ = ch;
    }

    return read;
}