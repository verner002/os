/**
 * @file terminal.c
 * @author verner002
 * @date 11/11/2025
*/

#include "kernel/terminal.h"
#include "kstdlib/stdio.h"
#include "kstdlib/stdlib.h"
#include "hal/driver.h"

#include "fs/file.h"

#include "hal/dev.h"
#include "kernel/mount.h"

extern void panic(void);
extern bool deffered_job;

/**
 * xatoi
*/

int32_t xatoi(char const *s) {
    if (!s)
        return 0;

    while (*s == ' ')
        ++s; // skip whitespaces

    int32_t value = 0;

    for (char c; ((c = *s) >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); ++s) {
        c -= '0';

        if (c > 9)
            c -= 'a' - '9' - 1;

        value = 16 * value + c;
    }

    return value;
}

/**
 * __path
*/

static void __path(struct dentry *node) {
    if (!node->parent) {
        printf(node->name);
        return;
    }

    __path(node->parent);
    printf("/%s", node->name);
}

/**
 * __tree
*/

static void __tree(struct dentry *node) {
    if (!strcmp(node->name, ".") || !strcmp(node->name, ".."))
        return;

    struct dentry *child = node->inode->child;

    while (child) {
        __path(child);
        putchar('\n');

        if (child->inode->child)
            __tree(child);

        child = child->next;
    }
}

int32_t wake_task = -1; // TODO: use list for "tasks to wake"

/**
 * __terminal_task
*/

void __terminal_task(void) {
    //while (!stdin);

    wake_task = __get_pid();
    printk("\033[33mterminal:\033[37m terminal daemon running, PID=%u\n", __get_pid());

    char *pwd = "/";

    while (1) {
        printf("[root@null %s]$ ", pwd);
        unsigned int size = 16; // base size
        unsigned int index = 0;
        char *input_buffer = (char *)kmalloc(sizeof(char) * size);

        char chr;

        while ((chr = getchar()) != EOF && chr != '\n') {
            if (index + 1 >= size) {
                size *= 2; //size = size * 1.5f + 0.5f; // growth factor
                input_buffer = (char *)krealloc(input_buffer, size);

                if (!input_buffer) {
                    printk("terminal: failed to reallocate input buffer\n");
                    __exit(-1);
                }
            }
            
            input_buffer[index++] = chr;
        }

        if (chr == EOF && !feof(stdin)) {
            printf("terminal: failed to read stdin\n");

            wake_task = -1;
            int32_t terminal_pid = __create_thread("terminal", (int32_t (*)(int argc, char **argv))&__terminal_task, THREAD_RING_0, THREAD_PRIORITY_LOW, NULL);

            if (terminal_pid == -1)
                printk("failed to start terminal\n");

            __exit(-1);
        }

        input_buffer[index] = '\0';

        char *strtok_buffer;
        char *cmd = strtok_r(input_buffer, " ", &strtok_buffer);

        if (!strcmp(cmd, "heap")) {
            //__dump_heap(); // at this point can lead to a deadlock
            printk("this function is disabled since it may cause dead-lock\n");
        } else if (!strcmp(cmd, "clear")) {
            __clear();
        } else if (!strcmp(cmd, "ps")) {
            __list_threads();
            //printk("this function is disabled since it may cause dead-lock\n");
        } else if (!strcmp(cmd, "e820")) {
            __e820_dump_mmap();
        } else if (!strcmp(cmd, "ls")) {
            //DIR *dir = opendir(pwd);
            //__fat12_list_rootdir();
            char *s;

            uint32_t mode = 0;

            struct dentry *child = current_dentry()->inode->child;

            while (s = strtok_r(NULL, " ", &strtok_buffer)) {
                if (!strcmp(s, "-l"))
                    mode = 1;
                else if (s) {
                    child = current_dentry();
                    child = child->d_ops->lookup(child, s);

                    if (!child) {
                        printf("ls: cannot access '%s': No such file or directory\n", s);
                        kfree(input_buffer);
                        continue;
                    }

                    child = child->inode->child;
                }
            }

            if (child) {
                int max_size = 0;

                struct dentry *temp = child;

                while (temp) {
                    if (temp->inode->size > max_size)
                        max_size = temp->inode->size;

                    temp = temp->next;
                }

                while (child) {
                    mode_t fmode = child->inode->mode;

                    if (mode == 1) {
                        putchar(fmode & 0x80000000 ? 'd' : '-');
                        putchar(fmode & 0400 ? 'r' : '-');
                        putchar(fmode & 0200 ? 'w' : '-');
                        putchar(fmode & 0100 ? 'x' : '-');
                        putchar(fmode & 0040 ? 'r' : '-');
                        putchar(fmode & 0020 ? 'w' : '-');
                        putchar(fmode & 0010 ? 'x' : '-');
                        putchar(fmode & 0004 ? 'r' : '-');
                        putchar(fmode & 0002 ? 'w' : '-');
                        putchar(fmode & 0001 ? 'x' : '-');
                        printf(" %s %s %*i ", "root", "root", digits(max_size), child->inode->size);
                    }

                    if ((fmode & 0777) == 0777)
                        printf("\033[30;42m");

                    if (fmode & 0x80000000)
                        printf("\033[94m");
                    else if (fmode & 0x40000000)
                        printf("\033[93m");

                    printf("%s\033[37;40m", child->name);

                    switch (mode) {
                        case 0:
                            printf("  ");
                            break;

                        case 1:
                            putchar('\n');
                            break;
                    }

                    child = child->next;
                }
                
                if (!mode)
                    putchar('\n');
            }
        } else if (!strcmp(cmd, "hexdump")) {
            char *address = strtok_r(NULL, " ", &strtok_buffer);

            if (!address) {
                printf("hexdump: expected address\n");
                kfree(input_buffer);
                continue;
            }
            
            char *count = strtok_r(NULL, " ", &strtok_buffer);

            if (!count) {
                printf("hexdump: expected count\n");
                kfree(input_buffer);
                continue;
            }
            uint32_t addr = xatoi(address);
            uint32_t max = atoi(count);

            printf("dumping %u byte(s) from %p (%u):\n", max, addr, addr);
            printf("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F %c 0123456789ABCDEF\n", (char)179);
            printf("------------------------------------------------------------------\n");

            for (uint32_t i = 0; i < (max / 16); ++i) {
                char buffer[16];

                for (uint32_t j = 0; j < 16; ++j) {
                    uint32_t index = 16 * i + j;

                    uint8_t data = ((uint8_t *)addr)[index];

                    buffer[j] = data;
                    printf("%02x ", data);
                }

                putchar((char)179);
                putchar(' ');

                for (uint32_t j = 0; j < 16; ++j) {
                    char ch = buffer[j];

                    switch (ch) {
                        case '\n':
                        case '\b':
                        case '\e':
                        case '\t':
                        case '\r':
                            putchar('?');
                            break;

                        default:
                            putchar(ch);
                            break;
                    }
                }

                putchar('\n');
            }

            if (max % 16) {
                char buffer[16];

                for (uint32_t i = 0; i < (max % 16); ++i) {
                    uint32_t index = (max & 0xfffffff0) + i;
                    uint8_t data = ((uint8_t *)addr)[i];

                    buffer[i] = data;
                    printf("%02x ", data);
                }

                for (uint32_t i = 0; i < (16 - (max % 16)); ++i) {
                    printf("   ");
                }

                putchar((char)179);
                putchar(' ');

                for (uint32_t i = 0; i < (max % 16); ++i) {
                    char ch = buffer[i];

                    switch (ch) {
                        case '\n':
                        case '\b':
                        case '\e':
                        case '\t':
                        case '\r':
                            putchar('?');
                            break;

                        default:
                            putchar(ch);
                            break;
                    }
                }

                putchar('\n');
            }
        } else if (!strcmp(cmd, "lsblk")) {
            struct device **list;
            int count;

            int result = get_devs(&list, &count);

            if (result) {
                printf("failed to retrieve devs list: %i\n", result);
                continue;
            }

            for (int i = 0; i < count; ++i) {
                struct device *device = list[i];
                kdev_t majmin = device->dev;
                printf("dev: id=%i, major=%u, minor=%u\n", i, MAJOR(majmin), MINOR(majmin));
            }

            //printk("NAME     DRIVER    TYPE      MOUNTPOINT\n");

            /*printf("NAME");

            uint32_t spaces;

            if (longest_dev_name < 5)
                spaces = 1;
            else
                spaces = longest_dev_name - 4;

            for (uint32_t i = 0; i < spaces; ++i)
                putchar(' ');

            printf("MAJ:MIN\n");

            for (uint32_t i = 0; i < devs_count; ++i)
                printf("%s %u:%u %s (address=%p)\n", devs[i].name, devs[i].major, devs[i].minor, devs[i].driver->module_name, devs[i].driver);*/
        } else if (!strcmp(cmd, "tree"))
            __tree(current_dentry());
        else if (!strcmp(cmd, "touch")) {
            char *name = strtok_r(NULL, " ", &strtok_buffer);

            struct dentry *root = current_dentry();
            struct dentry *home = root->d_ops->lookup(root, "/root");

            if (!home) {
                printf("/root not found\n");
                continue;
            }

            if (!create_file(home, name, 0, 0, 0))
                printf("touch: failed to create regular file\n");
        } else if (!strcmp(cmd, "mount")) {
            char *majs = strtok_r(NULL, " ", &strtok_buffer);
            char *mins = strtok_r(NULL, " ", &strtok_buffer);
            char *mountpoint = strtok_r(NULL, " ", &strtok_buffer);

            int maj = atoi(majs);
            int min = atoi(mins);

            int result = mount(MAJMIN(maj, min), mountpoint);

            if (result) {
                printk("Failed to mount device %u:%u to %s\n", maj, min, mountpoint);
                continue;
            }
        } else if (!strcmp(cmd, "ping")) {
            char *target = strtok_r(NULL, " ", &strtok_buffer);

            deffered_job = true;

            if (!target) {
                printf("ping: expected target\n");
                kfree(input_buffer);
                continue;
            }

            //ping(target);
        }
        else if (index)
            printf("terminal: %s: command not found\n", cmd);

        kfree(input_buffer);
    }
}