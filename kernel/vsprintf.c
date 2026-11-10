/**
 * @file vsprintf.h
 * @author verner002
 * @date 09/09/2026
*/

#include <string.h>

#include <kernel/vsprintf.h>

#define NUMBER_SMALL        1
#define NUMBER_SIGN         2

static int itoa(char *buf, int num, int base, int flags) {
    char *ptr = buf;
    int i = 0;
    
    char temp[((sizeof(int) * 8) * 1233 >> 12) + 1];

    char const *const digits =
        flags & NUMBER_SMALL ? "0123456789abcdef" : "0123456789ABCDEF";

    if (flags & NUMBER_SIGN && num < 0) {
        *ptr++ = '-';
        num = -num;
    }

    do
        temp[i++] = digits[num % (unsigned int)base];
    while (num /= (unsigned int)base);



    while (i-- > 0)
        *ptr++ = temp[i];

    return ptr - buf;
}

int vsprintf(char *buf, char const *fmt, va_list args) {
    char *ptr;
    char *s;
    size_t length;
    int flags;

    for (ptr = buf; *fmt; ++fmt) {
        if (*fmt != '%') {
            *ptr++ = *fmt;
            continue;
        }

        flags = 0;

        switch (*++fmt) {
            case '%':
                *ptr++ = '%';
                break;

            case 'c':
                *ptr++ = (char)va_arg(args, int);
                break;

            case 's':
                s = (char *)va_arg(args, char *);
                length = strlen(s);
                strcpy(ptr, s);
                ptr += length;
                break;

            case 'i':
                flags |= NUMBER_SIGN;
                __attribute__((fallthrough));

            case 'u':
                length = itoa(ptr, (int)va_arg(args, int), 10, flags);
                ptr += length;
                break;

            case 'p':
                unsigned int p = (unsigned int)va_arg(args, unsigned int);

                /**ptr++ = '0';
                *ptr++ = 'x';*/

                //length = itoa(ptr, (int)va_arg(args, int), 16, NUMBER_SMALL);
                for (size_t i = 0; i < (2 * sizeof(unsigned long)); ++i) {
                    char d = ((p = (p << 4) | (p >> 28)) & 0x0f) + '0';

                    // NOTE: use lookup table?
                    if (d > '9')
                        d += 'a' - '9' - 1;

                    *ptr++ = d;
                }
                
                //ptr += length;
                break;

            case 'x':
                flags |= NUMBER_SMALL;
                __attribute__((fallthrough));

            case 'X':
                length = itoa(ptr, (int)va_arg(args, int), 16, flags);
                ptr += length;
                break;

            default:
                if (!*fmt)
                    --fmt;

                *ptr++ = '?';
                break;
        }
    }

    *ptr = '\0';
    return ptr - buf;
}