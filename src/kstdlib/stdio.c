/**
 * Standard Input/Output
 * 
 * Author: verner002
 * 
 * TODO: use errno from errno.h(c?)
*/

/**
 * Includes
*/

#include "kstdlib/stdio.h"

/**
 * Static Global Variables
*/

static FILE
    _stdin = 0,
    _stdout = 1,
    _stderr = 2;
static unsigned int color = 0x07;

/**
 * Global Variables
*/

FILE
    *stdin = &_stdin,
    *stdout = &_stdout,
    *stderr = &_stderr;

/**
 * __stack_chk_fail
*/

void __stack_chk_fail(void) {
    printf("Stack overflow\n");
}

/**
 * putc
*/

int putc(int c, FILE *stream) {
    if (stream == stdout) return __putc((byte)c);
    else if (stream == stderr) return __putc((byte)c);
    
    return -1;
}

/**
 * putchar
*/

int putchar(int c) {
    return putc(c, stdout);
}

/**
 * vfprintf
 * 
 * TODO: implement escape sequences
*/

int vfprintf(FILE *stream, char const *s, va_list args) {
    char c;
    int errno = 0;

    for (unsigned int i = 0; !errno && (c = s[i]); ++i) {
        if (c == '%') {
            switch (/*c = */s[++i]) { // variable arguments
                case 'c': errno = putc(va_arg(args, int), stream); break;
                case 's': errno = fprintf(stream, va_arg(args, char const *)); break;
                case 'u': {
                    unsigned char n[10];
                    unsigned int u = va_arg(args, unsigned int);
                    unsigned int j = 0;

                    do n[j++] = u % 10 + '0'; while (u /= 10);
                    do errno = putc(n[--j], stream); while (j && !errno);
                    break;
                }
                case 'p': {
                    unsigned int p = va_arg(args, unsigned int);
                    //fprintf(stream, "0x%08x", p); -- implement!!!
                    putc('0', stream);
                    putc('x', stream);

                    for (unsigned int i = 0; i < sizeof(unsigned int) * 2; ++i) {
                        byte d = ((p = (p << 4) | (p >> 28)) & 0x0f) + '0';

                        if (d > '9') d += 'a' - '9' - 1;
                        
                        putc(d, stream);
                    }
                    break;
                }
                case 'l': {
                    switch (/*c = */s[++i]) {
                        case 'u': {
                            unsigned char n[20];
                            unsigned long u = va_arg(args, unsigned long);
                            unsigned int j = 0;

                            do n[j++] = u % 10 + '0'; while (u /= 10);
                            do errno = putc(n[--j], stream); while (j && !errno);
                            break;
                        }
                        default: return -1;
                    }
                    break;
                }
                default: return -1;
            }
        } else errno = putc(c, stream);
    }

    return errno;
}

/**
 * vprintf
*/

int vprintf(char const *s, va_list args) {
    return vfprintf(stdout, s, args);
}

/**
 * fprintf
*/

int fprintf(FILE *stream, char const *s, ...) {
    va_list args;
    va_start(args, s);

    int errno = vfprintf(stream, s, args);

    va_end(args);

    return errno;
}

/**
 * printf
*/

int printf(char const *s, ...) {
    va_list args;
    va_start(args, s);

    int errno = vprintf(s, args);

    va_end(args);

    return errno;
}

/**
 * puts
*/

int puts(char const *s) {
    return printf("%s\n\r", s);
}

/**
 * printk
*/

void printk(char const *s, ...) {
    va_list args;
    va_start(args, s);

    unsigned int padding;
    unsigned long ticks = __current_tick_count();

    printf("\033[32m[");

    if (ticks <= (unsigned long)999999999999) { // we can handle up to about 32 years
        unsigned int s = ticks / 1000;
        unsigned int ms = ticks % 1000;

        padding = 8 - log10(s);
        for (unsigned int i = 0; i < padding; ++i) putchar(' ');
        printf("%u.", s); 

        padding = 2 - log10(ms);
        for (unsigned int i = 0; i < padding; ++i) putchar('0');
        printf("%u", ms);
        
    } else printf("---------.---");

    printf("]\033[37m ");
    vprintf(s, args);
    va_end(args);
}