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
    _stdin,
    _stdout = (FILE){
    },
    _stderr = (FILE){
    };

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
 * feof
*/

bool feof(FILE *stream) {
    return stream->__flags & FILE_EOF;
}

/**
 * getc
 * 
 * NOTE:
 *  THIS IS A BLOCKING IMPLEMENTATION OF GETC
*/

//static bool __wake;

int getc(FILE *stream) {
    // buffer empty -> sleep the task
    if (!stream->__count) {
        /*__wake = FALSE;
        __wake_on(&__wake);*/
        __sleep_me();
    }

    /*if (!stream->__count) {
        // no data available
        errno = EAGAIN; // EWOULDBLOCK
        return -1;
    }*/

    --stream->__count;

    int c = *stream->__ptr;
    stream->__ptr = (stream->__ptr - stream->__base + 1) % stream->__size + stream->__base;
    return c;
}

/**
 * getchar
*/

int getchar(void) {
    return getc(stdin);
}

/**
 * putc
 * 
 * NOTE:
 *  THIS IS NON-CANONICAL IMPLEMENTATION OF PUTC
*/

int32_t putc(int c, FILE *stream) {
    if (stream == stdout)
        return __putc((uint8_t)c);
    else if (stream == stderr)
        return __putc((uint8_t)c);
    else if (stream == stdin) {
        if (c == '\b') {
            if (stream->__count) {
                --stream->__count;
                --stream->__index;
            }

            return 0;
        } else if (stream->__count < stream->__size) {
            ++stream->__count;
            stream->__base[stream->__index++ % stream->__size] = c;

            /*if (c == '\n')
                __wake = TRUE;*/

            return 0;
        }
    }
    
    return -1;
}

/**
 * putchar
*/

int32_t putchar(int c) {
    return putc(c, stdout);
}

/**
 * vfprintf
 * 
 * TODO: implement escape sequences
*/

int32_t vfprintf(FILE *stream, char const *s, va_list args) {
    char c;
    int errno = 0;

    for (uint32_t i = 0; !errno && (c = s[i]); ++i) {
        if (c == '%') {
            switch (/*c = */s[++i]) { // variable arguments
                case 'c': errno = putc(va_arg(args, int), stream); break;
                case 's': errno = fprintf(stream, va_arg(args, char const *)); break;
                case '.': {
                    switch (s[++i]) {
                        case '*': {
                            uint32_t limit = va_arg(args, uint32_t);

                            switch (s[++i]) {
                                case 's': {
                                    char const *string = va_arg(args, char const *);

                                    for (uint32_t j = 0; j < limit; ++j)
                                        putchar(string[j]);
                                    break;
                                }

                                default:
                                    return -1;
                            }
                            break;
                        }

                        default:
                            return -1;
                    }
                    break;
                }
                case 'u': {
                    uint8_t n[10];
                    uint32_t u = va_arg(args, uint32_t);
                    uint32_t j = 0;

                    do n[j++] = u % 10 + '0'; while (u /= 10);
                    do errno = putc(n[--j], stream); while (j && !errno);
                    break;
                }
                case 'p': {
                    uint32_t p = va_arg(args, uint32_t);
                    //fprintf(stream, "0x%08x", p); -- implement!!!
                    putc('0', stream);
                    putc('x', stream);

                    for (uint32_t j = 0; j < sizeof(uint32_t) * 2; ++j) {
                        uint8_t d = ((p = (p << 4) | (p >> 28)) & 0x0f) + '0';

                        if (d > '9') d += 'a' - '9' - 1;
                        
                        putc(d, stream);
                    }
                    break;
                }
                case 'l': {
                    switch (/*c = */s[++i]) {
                        case 'u': {
                            uint8_t n[20];
                            uint64_t u = va_arg(args, uint64_t);
                            uint32_t j = 0;

                            do n[j++] = u % 10 + '0'; while (u /= 10);
                            do errno = putc(n[--j], stream); while (j && !errno);
                            break;
                        }
                        default:
                            return -1;
                    }
                    break;
                }
                case '0': { // TODO: use variable arguments, if (s[i] >= '0' && s[i] <= '9')
                    switch (s[++i]) {
                        case '2': {
                            switch (s[++i]) {
                                case 'x': {
                                    uint8_t n = va_arg(args, uint32_t);

                                    for (uint32_t j = 0; j < sizeof(uint8_t) * 2; ++j) {
                                        // n = rotate_left(n)
                                        uint8_t d = ((n = (n << 4) | (n >> 4)) & 0x0f) + '0';

                                        if (d > '9')
                                            d += 'a' - '9' - 1;
                                        
                                        putc(d, stream);
                                    }
                                    break;
                                }
                                default:
                                    return -1;
                            }
                            break;
                        }

                        case '4': {
                            switch (s[++i]) {
                                case 'x': {
                                    uint16_t n = va_arg(args, uint32_t);

                                    for (uint32_t j = 0; j < sizeof(uint16_t) * 2; ++j) {
                                        // n = rotate_left(n)
                                        uint16_t d = ((n = (n << 4) | (n >> 12)) & 0x0f) + '0';

                                        if (d > '9')
                                            d += 'a' - '9' - 1;
                                        
                                        putc(d, stream);
                                    }
                                    break;
                                }
                                default:
                                    return -1;
                            }
                            break;
                        }

                        case '8': {
                            switch (s[++i]) {
                                case 'x': {
                                    uint32_t n = va_arg(args, uint32_t);

                                    for (uint32_t j = 0; j < sizeof(uint32_t) * 2; ++j) {
                                        // n = rotate_left(n)
                                        uint32_t d = ((n = (n << 4) | (n >> 28)) & 0x0f) + '0';

                                        if (d > '9')
                                            d += 'a' - '9' - 1;
                                        
                                        putc(d, stream);
                                    }
                                    break;
                                }
                                default:
                                    return -1;
                            }
                            break;
                        }
                        default:
                            return -1;
                    }
                    break;
                }
                default:
                    return -1;
            }
        } else errno = putc(c, stream);
    }

    return errno;
}

/**
 * vprintf
*/

int32_t vprintf(char const *s, va_list args) {
    return vfprintf(stdout, s, args);
}

/**
 * fprintf
*/

int32_t fprintf(FILE *stream, char const *s, ...) {
    va_list args;
    va_start(args, s);

    int errno = vfprintf(stream, s, args);

    va_end(args);

    return errno;
}

/**
 * printf
 * 
 * FIXME: printf mutex can cause dead-lock at
 *  certain circumstances
*/

int32_t printf(char const *s, ...) {
    static bool printf_mutex = FALSE;
    __mutex_lock(&printf_mutex);
    
    va_list args;
    va_start(args, s);

    int errno = vprintf(s, args);

    va_end(args);

    __mutex_unlock(&printf_mutex);
    return errno;
}

/**
 * puts
*/

int32_t puts(char const *s) {
    return printf("%s\n\r", s);
}

/**
 * printk
*/

void printk(char const *s, ...) {
    static bool printk_mutex = FALSE;
    __mutex_lock(&printk_mutex);

    va_list args;
    va_start(args, s);

    uint32_t padding;
    uint64_t ticks = __current_tick_count();

    printf("\033[32m[");

    if (ticks <= (uint64_t)999999999999) { // we can handle up to about 32 years
        uint32_t s = ticks / 1000;
        uint32_t ms = ticks % 1000;

        padding = 8 - log10(s);
        for (uint32_t i = 0; i < padding; ++i) putchar(' ');
        printf("%u.", s); 

        padding = 2 - log10(ms);
        for (uint32_t i = 0; i < padding; ++i) putchar('0');
        printf("%u", ms);
        
    } else printf("---------.---");

    printf("]\033[37m ");
    vprintf(s, args);
    va_end(args);

    __mutex_unlock(&printk_mutex);
}

/**
 * fopen
*/

FILE *fopen(char const *path, char const *mode) {

}

/**
 * fseek
*/

int32_t fseek(FILE *file, uint32_t offset, uint8_t mode) {

}

/**
 * fread
*/

int32_t fread(FILE *file, char *buffer, uint32_t count) {

}

/**
 * fwrite
*/

int32_t fwrite(FILE *file, char const *buffer, uint32_t count) {

}

/**
 * fclose
*/

int32_t fclose(FILE *file) {
    
}

/**
 * opendir
*/

DIR *opendir(char const *path, char const *mode) {

}