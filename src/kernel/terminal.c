/**
 * @file terminal.c
 * @author verner002
 * @date 11/11/2025
 * 
 * TODO: this code is used just for testing features
 *  not an official terminal
*/

#include "kernel/terminal.h"
#include "kstdlib/stdio.h"
#include "kstdlib/stdlib.h"
#include "hal/driver.h"

#include "fs/file.h"

#include "hal/device.h"
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

    if (node->parent && node->parent->parent)
        putchar('/');

    printf("%s", node->name);
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

char const *input() {
    unsigned int size = 16; // base size
    unsigned int index = 0;
    char *buffer = (char *)kmalloc(sizeof(char) * size);

    if (!buffer)
        return NULL;

    char chr;

    while (((chr = getchar()) != EOF) && chr != '\n') {
        if (index + 1 >= size) {
            size *= 2; //size = size * 1.5f + 0.5f; // growth factor
            buffer = (char *)krealloc(buffer, size);

            if (!buffer)
                return NULL;
        }
        
        buffer[index++] = chr;
    }

    buffer[index] = '\0';

    int length = strlen(buffer) + sizeof(char);
    char *string = (char *)kmalloc(length);

    if (!string) {
        kfree(buffer);
        return NULL;
    }
    
    strcpy(string, buffer);
    kfree(buffer);
    return string;
}

extern struct dentry root_dentry;

int __terminal_task(int argc, char **argv) {
    //while (!stdin);

    wake_task = __get_pid();
    printk("\033[33mterminal:\033[37m terminal daemon running, PID=%u\n", __get_pid());

    char const *login;
    char const *password;
    int result;

    do {
        do {
            printf("login: ");
            login = input();
        } while (!login || !*login);

        do {
            printf("password: ");
            password = input();
        } while (!password || !*password);

        result = user_login(login, password);

        if (result)
            printf("Invalid login or password\n");
    } while (result);

    char *pwd = "/";

    while (1) {
        printf("[root@%s %s]$ ", argv[0], pwd);
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

            struct dentry *child = current_dentry();

            bool error = false;

            while (!error && (s = strtok_r(NULL, " ", &strtok_buffer))) {
                if (!strcmp(s, "-l"))
                    mode = 1;
                else if (s) {    
                    char *path = s;
                    
                    if (*s == '/') {
                        ++s;
                        child = &root_dentry;
                    } else
                        child = current_dentry();
                    
                    errno = 0;
                    child = child->d_ops->lookup(child, s);

                    if (errno == ENOENT) {
                        // no entry found
                        printf("ls: cannot access '%s': No such file or directory\n", path);
                        error = true;
                    } else if (errno == ENOTDIR || (!(child->inode->mode & S_IFDIR) && s[strlen(s) - 1] == '/')) {
                        // entry is not a directory or
                        // there is a slash at the end
                        // of a file name
                        printf("ls: cannot access '%s': Not a directory\n", path);
                        error = true;
                    }
                }
            }

            if (error) {
                kfree(input_buffer);
                continue;
            }

            if (child->inode->mode & S_IFDIR) {
                child = child->inode->child;

                if (child) {
                    uint32_t max_size = 0;

                    struct dentry *temp = child;

                    while (temp) {
                        if (((temp->inode->mode & S_IFCHR) || (temp->inode->mode & S_IFBLK)) && 6 > max_size)
                            max_size = 9999999;
                        else if (temp->inode->size > max_size)
                            max_size = temp->inode->size;

                        temp = temp->next;
                    }

                    while (child) {
                        mode_t fmode = child->inode->mode;

                        if (mode == 1) {
                            if (fmode & S_IFDIR)
                                putchar('d');
                            else if (fmode & S_IFCHR)
                                putchar('c');
                            else if (fmode & S_IFBLK)
                                putchar('b');
                            else
                                putchar('-');

                            putchar(fmode & S_IRUSR ? 'r' : '-');
                            putchar(fmode & S_IWUSR ? 'w' : '-');
                            putchar(fmode & S_IXUSR ? 'x' : '-');
                            putchar(fmode & S_IRGRP ? 'r' : '-');
                            putchar(fmode & S_IWGRP ? 'w' : '-');
                            putchar(fmode & S_IXGRP ? 'x' : '-');
                            putchar(fmode & S_IROTH ? 'r' : '-');
                            putchar(fmode & S_IWOTH ? 'w' : '-');
                            putchar(fmode & S_IXOTH ? 'x' : '-');

                            printf(" %s ", "root");

                            if ((fmode & S_IFCHR) || (fmode & S_IFBLK)) {
                                printf("%s %*i %*i", "disk", 3, MAJOR(child->inode->kdev), 3, MINOR(child->inode->kdev));
                            } else
                                printf("%s %*i", "root", digits(max_size), child->inode->size);

                            putchar(' ');
                        }

                        if ((fmode & 0777) == 0777)
                            printf("\033[30;42m");

                        if (fmode & S_IFDIR)
                            printf("\033[94m");
                        else if ((fmode & S_IFCHR) || (fmode & S_IFBLK))
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
            } else {
                printf("%s\n", child->name);
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
                    //uint32_t index = (max & 0xfffffff0) + i;
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

            printf("NAME MAJ:MIN MOUNTPOINTS\n");

            int max_name = 4;
            int max_majmin = 7;

            for (int i = 0; i < count; ++i) {
                struct device *device = list[i];
                
                if (strlen(device->type->name) > max_name)
                    max_name = strlen(device->type->name);
                
                if ((log10(MAJOR(device->dev)) + log10(MINOR(device->dev)) + 3) > max_majmin)
                    max_majmin = log10(MAJOR(device->dev)) + log10(MINOR(device->dev)) + 3;
            }

            for (int i = 0; i < count; ++i) {
                struct device *device = list[i];
                kdev_t majmin = device->dev;
                printf("%*s %*i:%i %s\n", max_name, device->type->name, max_majmin - log10(MINOR(device->dev)) - 2, MAJOR(device->dev), MINOR(device->dev), lookup_mountpoint(majmin));
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
                kfree(input_buffer);
                continue;
            }

            if (!create_file(home, name, 0, 0, 0))
                printf("touch: failed to create regular file\n");
        } else if (!strcmp(cmd, "mount")) {
            char *device_path = strtok_r(NULL, " ", &strtok_buffer);
            char *mountpoint = strtok_r(NULL, " ", &strtok_buffer);

            mount(device_path, mountpoint);
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