/**
 * @file stdio.c
 * @author verner002
 * @date 15/02/2026
 * 
 * TODO: use errno from errno.h(c?)
*/

#include "kstdlib/stdio.h"
#include "drivers/graphics/graphix.h"

#define DIGITS_COUNT(x) (((x) * 8 * 1233 >> 12) + 1)

FILE
    *stdin = NULL,
    *stdout = NULL,
    *stderr = NULL;

/**
 * __stack_chk_fail
*/

void __stack_chk_fail(void) {
    printf("Stack overflow %u\n", __get_pid());
}

/**
 * feof
*/

int feof(FILE *stream) {
    return stream->__flags & FILE_EOF;
}

/**
 * getc
 * 
 * NOTE:
 *  THIS IS A BLOCKING IMPLEMENTATION OF GETC
*/

int getc(FILE *stream) {
    __mutex_lock(&stream->__lock);

    // buffer empty -> sleep the task
    if (!stream->__count) {
        stream->__ready = false;
        __mutex_unlock(&stream->__lock);
        __wake_on(&stream->__ready);
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

int internal_putc(int c, FILE *stream) {
    if (!stream)
        return __putc(c); //__graphix_putc(c);

    if (c == '\b') {
        if (stream->__count) {
            --stream->__count;
            --stream->__index;
        }

        //__mutex_unlock(&stream->__lock);
        return 0;
    }

    // is there a space in stream to
    // write data? if no, let other
    // tasks read the stream
    while (stream->__count >= stream->__size) {
        // we unlock the stream so that other threads
        // can read from it
        // RFC: use SHARE_READ instead?
        __mutex_unlock(&stream->__lock);
        __yield();
        // and lock the stream again
        __mutex_lock(&stream->__lock);
    }
    
    /*else if (stream->__count < stream->__size) {*/
        ++stream->__count;
        stream->__base[stream->__index++ % stream->__size] = c;

        if (/*c == '\0' ||*/ c == '\n')
            stream->__ready = true; // TODO: WAKE UP THE SLEEPING PROCESS

    return 0;
}

/**
 * putc
 * 
 * NOTE:
 *  THIS IS NON-CANONICAL IMPLEMENTATION OF PUTC
*/

int putc(int c, FILE *stream) {
    if (!stream)
        return __putc(c); //__graphix_putc(c);

    __mutex_lock(&stream->__lock);

    int error = internal_putc(c, stream);

        __mutex_unlock(&stream->__lock);
        return error;
    /*}
    
    __mutex_unlock(&stream->__lock);
    // wait till buffer is read instead
    // returning -1?
    return -1;*/
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
 * TODO: merge duplicate code for int a unsigned int
*/

int __fprintf(FILE *stream, char const *s, ...);

int __vfprintf(FILE *stream, char const *str, va_list args) {
    int err = 0;
    char c;
    int i = 0;

    while ((c = str[i++]) && !err) {
        switch (c) {
            case '%': {
                bool lp;

                if (str[i] == '*') {
                    lp = true;
                    ++i;
                } else
                    lp = false;

                switch (str[i++]) {
                    case '%': // '%' character
                        err = internal_putc('%', stream);
                        break;

                    case 'c': // character
                        err = internal_putc(va_arg(args, int), stream);
                        break;

                    case 's': // string
                        err = __fprintf(stream, va_arg(args, char const *));
                        break;

                    case 'i': { // integer
                        int length;

                        if (lp)
                            length = va_arg(args, int);

                        int value = va_arg(args, int);

                        if (lp) {
                            int spaces = length - digits(value);

                            for (int s = 0; s < spaces; ++s)
                                internal_putc(' ', stream);
                        }

                        unsigned int nvalue;

                        if (value < 0) {
                            err = internal_putc('-', stream);

                            // neg dword [value]
                            nvalue = (unsigned int)-value;
                        } else
                            nvalue = (unsigned int)value;

                        // efficient only when we print '-'
                        if (err)
                            break;

                        // ((sizeof(int) * 8) * 1233 >> 12) + 1
                        int digits[DIGITS_COUNT(sizeof(int))];
                        int j = 0;

                        do
                            digits[j++] = nvalue % 10 + '0';
                        while (nvalue /= 10);

                        do {
                            err = internal_putc(digits[--j], stream);

                            if (err)
                                break;
                        } while (j);

                        break;
                    }

                    case 'u': { // unsigned integer
                        unsigned int value = va_arg(args, unsigned int);

                        int digits[DIGITS_COUNT(sizeof(unsigned int))];
                        int j = 0;

                        do
                            digits[j++] = value % 10 + '0';
                        while (value /= 10);

                        do {
                            err = internal_putc(digits[--j], stream);

                            if (err)
                                break;;
                        } while (j);

                        break;
                    }

                    case 'p': { // pointer
                        unsigned int value = va_arg(args, unsigned int);

                        err = internal_putc('0', stream);

                        if (err)
                            break;

                        err = internal_putc('x', stream);
                        
                        if (err)
                            break;

                        for (uint32_t j = 0; j < 2 * sizeof(unsigned int); ++j) {
                            uint8_t d = ((value = (value << 4) | (value >> 28)) & 0x0f) + '0';

                            // RFC: use lookup table?
                            if (d > '9')
                                d += 'a' - '9' - 1;
                            
                            err = internal_putc(d, stream);
                            
                            if (err)
                                break;
                        }
                        
                        break;
                    }

                    case '0': {
                        unsigned int value = va_arg(args, unsigned int);
                        int digits;

                        switch (str[i++]) {
                            case '2': digits = sizeof(uint8_t); break;
                            case '4': digits = sizeof(uint16_t); break;
                            case '8': digits = sizeof(uint32_t); break;
                            default: digits = -1; break;
                        }

                        if (digits == -1) {
                            err = -1;
                            break;
                        }

                        int gap_const;

                        switch (str[i++]) {
                            case 'x': gap_const = 'a' - '9' - 1; break;
                            case 'X': gap_const = 'A' - '9' - 1; break;
                            default: gap_const = -1; break;
                        }

                        if (gap_const == -1) {
                            err = -1;
                            break;
                        }

                        digits *= 2;

                        for (int j = 0; j < digits; ++j) {
                            value = (value << 4) | (value >> 4 * digits - 4);
                            int digit = (value & 15) + '0';

                            if (digit > '9')
                                digit += gap_const;

                            err = internal_putc(digit, stream);

                            if (err)
                                break;
                        }

                        break;
                    }

                    default:
                        err = -1;
                        break;
                }
                break;
            }

            default:
                err = internal_putc(c, stream);
                break;
        }
    }

    return err;
}

/**
 * vfprintf
*/

int vfprintf(FILE *stream, char const *str, va_list args) {
    if (stream)
        __mutex_lock(&stream->__lock);

    int error = __vfprintf(stream, str, args);

    if (stream)
        __mutex_unlock(&stream->__lock);

    return error;
}

/**
 * vprintf
*/

int vprintf(char const *s, va_list args) {
    return vfprintf(stdout, s, args);
}

int __fprintf(FILE *stream, char const *s, ...) {
    va_list args;
    va_start(args, s);

    int error = __vfprintf(stream, s, args);

    va_end(args);
    return error;
}

/**
 * fprintf
*/

int fprintf(FILE *stream, char const *s, ...) {
    if (stream)
        __mutex_lock(&stream->__lock);

    va_list args;
    va_start(args, s);

    int error = __vfprintf(stream, s, args);

    va_end(args);

    if (stream)
        __mutex_unlock(&stream->__lock);
    return error;
}

/**
 * printf
*/

int printf(char const *s, ...) {
    va_list args;
    va_start(args, s);

    int error = vprintf(s, args);

    va_end(args);
    return error;
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

    uint32_t padding;
    uint64_t ticks = __current_tick_count();

    __fprintf(NULL, "\033[32m[");

    // we can handle up to about 32 years
    if (ticks <= (uint64_t)999999999999) {
        uint32_t s = ticks / 1000;
        uint32_t ms = ticks % 1000;

        padding = 8 - log10(s);
        
        for (uint32_t i = 0; i < padding; ++i)
            internal_putc(' ', NULL);
        
            __fprintf(NULL, "%u.", s); 

        padding = 2 - log10(ms);
        
        for (uint32_t i = 0; i < padding; ++i)
            internal_putc('0', NULL);
        
            __fprintf(NULL, "%u", ms);
        
    } else
        __fprintf(NULL, "---------.---");

    __fprintf(NULL, "]\033[37m ");
    __vfprintf(NULL, s, args);
    va_end(args);
}