/**
 * @file config.c
 * @author verner002
 * @date 02/09/2025
*/

#include "kernel/config.h"
#include "kstdlib/string.h"
#include "kernel/kdev.h"
#include "kstdlib/stdio.h"

extern kdev_t root_dev;
extern char *envs[16];

/**
 * parse_config
 * 
 * TODO: implement some kind of `__eat_line' function
 *  for error reporting
*/

int parse_config(char const *config) {
    uint32_t line = 1;
    uint32_t column = 1;
    char *ptr = config;

    while (*ptr) {
        char *origin = ptr;

        switch (*ptr) {
            case '\n':
                // end-of-line
                ++ptr;
                column = 1;
                ++line;
                break;

            case 'a' ... 'z': {
                // configuration
                while (*ptr >= 'a' && *ptr <= 'z')
                    ++ptr;

                if (*ptr != '=') {
                    column += ptr - origin;

                    while (*ptr != '\n' && *ptr != '\0')
                        ++ptr;

                    printk("config:%u:%u: expected assignment `='\n", line, column);
                    break;
                }

                ++ptr;

                if (!strncmp(origin, "root", 4)) {
                    /*if (root_dev != NO_DEV)
                        printk("overwritting root dev\n");
                    else
                        printk("configuring root\n");*/
                } /*else if (!strncmp(origin, "echo", 4)) {
                    column += ptr - origin;

                    while (*ptr != '\n' && *ptr != '\0')
                        putchar(*ptr++);
                } */else {
                    column += ptr - origin;

                    while (*ptr != '\n' && *ptr != '\0')
                        ++ptr;

                    printk("config:%u:%u: unknown property\n", line, column);
                    break;
                }

                if (strncmp(ptr, "/dev/", 5)) {
                    while (*ptr != '\n' && *ptr != '\0')
                        ++ptr;

                    printk("config:%u:%u: expected dev path /dev/xxx\n", line, column);
                    break;
                }

                ptr += 5;

                char *dev_name = ptr;

                // dev name (lower-case chars and digits)
                while ((*ptr >= 'a' && *ptr <= 'z') || (*ptr >= '0' && *ptr <= '9') || *ptr == '-' || *ptr == '_')
                    ++ptr;

                if (ptr == dev_name) {
                    column += ptr - origin;

                    while (*ptr != '\n' && *ptr != '\0')
                        ++ptr;

                    printk("config:%u:%u: expected root device name\n", line, column);
                    break;
                }

                if (*ptr != '\n' && *ptr != '\0') {
                    column += ptr++ - origin; // we know that current char is not new-line nor end-of-line

                    while (*ptr != '\n' && *ptr != '\0')
                        ++ptr;

                    printk("config:%u:%u: expected end of line or end of file\n", line, column);
                    break;
                }

                if (*ptr == '\n') {
                    *ptr = '\0'; // replace new-line with end-of-line for dev to kdev conversion
                    ++ptr;
                    column = 1;
                    ++line;
                } else
                    column += ptr - origin;

                root_dev = name2kdev(dev_name);
                break;
            }

            case 'A' ... 'Z':
                // environment variable
                while (*ptr != '\n' && *ptr != '\0')
                    ++ptr;

                printk("config:%u:%u: environment variables parsing not implemented\n", line, column);
                break;

            default:
                while (*ptr != '\n' && *ptr != '\0')
                    ++ptr;

                printk("config:%u:%u unexpected symbol\n", line, column);
                break;
        }
    }

    return 0;
}