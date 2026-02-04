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
#include "drivers/graphics/graphix.h"

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
    printf("Stack overflow %u\n", __get_pid());
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

bool __wake;

int getc(FILE *stream) {
    __mutex_lock(&stream->__lock);

    // buffer empty -> sleep the task
    if (!stream->__count) {
        __wake = false;
        __mutex_unlock(&stream->__lock);
        __wake_on(&__wake);
        __mutex_lock(&stream->__lock);
    }

    /*if (!stream->__count) {
        // no data available
        errno = EAGAIN; // EWOULDBLOCK
        return -1;
    }*/

    --stream->__count;

    int c = *stream->__ptr;
    stream->__ptr = (stream->__ptr - stream->__base + 1) % stream->__size + stream->__base;
    __mutex_unlock(&stream->__lock);
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
    if (stream == &_stdout)
        return __putc(c); //__graphix_putc(c);

    __mutex_lock(&stream->__lock);

    if (c == '\b') {
        if (stream->__count) {
            --stream->__count;
            --stream->__index;
        }

        __mutex_unlock(&stream->__lock);
        return 0;
    }

    // is there a space in stream to
    // write data? if no, let other
    // tasks read the stream
    while (stream->__count >= stream->__size) {
        __mutex_unlock(&stream->__lock);
        __yield();
        __mutex_lock(&stream->__lock);
    }
    
    /*else if (stream->__count < stream->__size) {*/
        ++stream->__count;
        stream->__base[stream->__index++ % stream->__size] = c;

        if (c == '\n')
            __wake = true;

        __mutex_unlock(&stream->__lock);
        return 0;
    /*}
    
    __mutex_unlock(&stream->__lock);
    // wait till buffer is read instead
    // returning -1?
    return -1;*/
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
    int error = 0;

    for (uint32_t i = 0; !error && (c = s[i]); ++i) {
        if (c == '%') {
            switch (/*c = */s[++i]) { // variable arguments
                case 'c':
                    error = putc(va_arg(args, int), stream);
                    break;
                
                case 's':
                    error = fprintf(stream, va_arg(args, char const *));
                    break;
                
                case '.': {
                    switch (s[++i]) {
                        case '*': {
                            uint32_t limit = va_arg(args, uint32_t);

                            switch (s[++i]) {
                                case 's': {
                                    char const *string = va_arg(args, char const *);

                                    for (uint32_t j = 0; j < limit; ++j)
                                        putc(string[j], stdout);

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
                    do error = putc(n[--j], stream); while (j && !error);
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
        } else error = putc(c, stream);
    }

    return error;
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

    int error = vfprintf(stream, s, args);

    va_end(args);

    return error;
}

/**
 * printf
 * 
 * FIXME: printf mutex can cause dead-lock at
 *  certain circumstances
*/

int32_t printf(char const *s, ...) {
    static bool printf_mutex = false;
    __mutex_lock(&printf_mutex);
    
    va_list args;
    va_start(args, s);

    int error = vprintf(s, args);

    va_end(args);

    __mutex_unlock(&printf_mutex);
    return error;
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
    static bool printk_mutex = false;
    __mutex_lock(&printk_mutex);

    va_list args;
    va_start(args, s);

    uint32_t padding;
    uint64_t ticks = __current_tick_count();

    fprintf(&_stdout, "\033[32m[");

    // we can handle up to about 32 years
    if (ticks <= (uint64_t)999999999999) {
        uint32_t s = ticks / 1000;
        uint32_t ms = ticks % 1000;

        padding = 8 - log10(s);
        
        for (uint32_t i = 0; i < padding; ++i)
            putc(' ', &_stdout);
        
            fprintf(&_stdout, "%u.", s); 

        padding = 2 - log10(ms);
        
        for (uint32_t i = 0; i < padding; ++i)
            putc('0', &_stdout);
        
            fprintf(&_stdout, "%u", ms);
        
    } else
        fprintf(&_stdout, "---------.---");

    fprintf(&_stdout, "]\033[37m ");
    vfprintf(&_stdout, s, args);
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