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
    if (stream == stdout) return __putc((byte)c, 0x00);
    else if (stream == stderr) return __putc((byte)c, 0x04);
    
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

    unsigned char color = 0x0f; // black background, white foreground 

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
        } /*else if (c == '\033') { // escape sequences
            if (s[++i] != '[') return -1;

            unsigned p = 0;
            unsigned n = 0;

            do {
                c = s[++i];

                if (c < '0' || c > '9') return -1; // we expect a digit

                do {
                    n = n * 10 + c - '0';
                    c = s[++i];
                } while (c >= '0' && c <= '9');

                if (!p && n) return -1; // TODO: implement other options
                else if (p == 1) {
                    switch (n) {
                        case 30: color = 0x00; break;
                        case 31: color = 0x04; break;
                        case 32: color = 0x0a; break;
                        case 33: color = 0x0e; break;
                        case 34: color = 0x09; break;
                        case 35: color = 0x0d; break;
                        case 36: color = 0x0b; break;
                        case 37: color = 0x0f; break;
                        default: return -1;
                    }
                }

                if (c != 'm' && c != ';') return -1;

                ++p;
            } while (c == ';');

        }*/ else errno = putc(c, stream);
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

    if (ticks > 0x00000000ffffffff) {
        printf("[-------.---");
    } else {
        unsigned int s = ticks / 1000;
        unsigned int ms = ticks % 1000;

        putchar('[');

        padding = 6 - log10(s);
        for (unsigned int i = 0; i < padding; ++i) putchar(' ');
        printf("%u.", s); 

        padding = 2 - log10(ms);
        for (unsigned int i = 0; i < padding; ++i) putchar('0');
        printf("%u", ms);
    }

    printf("] ");
    vprintf(s, args);
    va_end(args);
}