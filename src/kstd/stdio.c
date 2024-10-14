/**
 * Standard Input/Output
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "kstd/stdio.h"

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
*/

int vfprintf(FILE *stream, char const *s, va_list args) {
    char c;
    int errno = 0;

    for (unsigned int i = 0; !errno && (c = s[i]); ++i) {
        if (c == '%') {
            c = s[++i];

            switch (c) {
                case 'c': errno = putc(va_arg(args, int), stream); break;
                case 's': errno = fprintf(stream, va_arg(args, char const *)); break;
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