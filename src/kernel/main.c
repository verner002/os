/**
 * @file main.c
 * @author verner002
 * @date 11/10/2024
 * @note j+k=<3
*/

#define CONFIG_FDC

/**
 * Includes
*/

#include "null.h"

#include "drivers/cpu.h"
#include "drivers/power/acpi.h"
#include "drivers/8259a.h"
#include "drivers/8254a.h"
#include "drivers/char/8042.h"
#include "drivers/dma/8237a.h"
#include "drivers/block/82077aa.h"

#include "hal/driver.h"
#include "hal/vfs.h"

//#include "kernel/syms.h"
#include "mm/e820.h"
#include "mm/pager.h"
#include "kernel/ts.h"
#include "kernel/task.h"
#include "fs/fat12/fat12.h"

#include "kstdlib/errno.h"
#include "kstdlib/stdio.h"
#include "kstdlib/stdlib.h"
#include "mm/heap.h"

#include "kernel/sysfs.h"
#include "hal/bus.h"
#include "drivers/bus/pci.h"
#include "drivers/block/ide.h"
#include "kernel/kdev.h"
#include "kernel/config.h"

#include "kernel/tty.h"
#include "kernel/mount.h"

/**
 * panic
*/

void panic(void) {
    printk("\033[31mKERNEL PANIC\n");

    for (;;) {
        asm volatile (
            "cli\n\t"
            "hlt"
        );
    }
}

__attribute__((interrupt)) static void __spurious_irq_isr(INTERRUPT_FRAME *frame) {
    
}

__attribute__((interrupt)) static void __default_isr(INTERRUPT_FRAME *frame) {
    printk("warning: unhandled interrupt!\n");
    for (;;);
}

__attribute__((interrupt)) static void __division_by_zero(INTERRUPT_FRAME *frame) {
    printk("fault: division by zero!\n");
    for (;;);
}

__attribute__((interrupt)) static void __nmi(INTERRUPT_FRAME *frame) {
    printk("info: nmi!\n");
    for (;;);
}

__attribute__((interrupt)) static void __overflow(INTERRUPT_FRAME *frame) {
    printk("trap: overflow!\n");
    for (;;);
}

__attribute__((interrupt)) static void __bounds_check(INTERRUPT_FRAME *frame) {
    printk("fault: bounds check!\n");
    for (;;);
}

__attribute__((interrupt)) static void __invalid_opcode(INTERRUPT_FRAME *frame) {
    uint32_t eip; // instruction that caused the fault
    
    asm volatile (
        "pop %0"
        : "=m" (eip)
        :
        :
    );

    printk("fault: invalid opcode at %p!\n", eip);

    if (thread_lcurrent)
        printk("thread %u\n", __get_pid());

    for (;;);
}

__attribute__((interrupt)) static void __device_not_available(INTERRUPT_FRAME *frame) {
    printk("fault: device not available!\n");
    for (;;);
}

__attribute__((interrupt)) static void __double_fault(INTERRUPT_FRAME *frame) {
    printk("abort: double fault!\n");
    for (;;);
}

__attribute__((interrupt)) static void __general_protection_fault(INTERRUPT_FRAME *frame) {
    if (thread_lcurrent && __get_pid() != 0) {
        __enable_interrupts();
        __exit(1); // no return
    }
    
    printk("fault: general protection fault!\n");
    for (;;);
}

__attribute__((interrupt)) static void __page_fault(INTERRUPT_FRAME *frame) {
    // TODO: TEST THIS CODE!!!
    uint32_t vas;

    asm volatile (
        "mov eax, cr2"
        : "=a" (vas)
        :
        :
    );

    printk("page-fault when accessing address %p\n", vas);

    vas &= 0xfffff000;

    /*void *pas = __e820_rmalloc(4096, TRUE);

    if (!pas) {
        printk("kernel: page-fault: out of memory\n");
        panic();
    }*/

    if (__map_page(vas, vas, PAGE_READ_WRITE)) {
        printk("kernel: page-fault: map failed\n");
        panic();
    }

    panic();
}

/*__attribute__((interrupt)) void __pit_irq0_handler(INTERRUPT_FRAME *frame) {
    __update_tick_counter();
    __send_eoi(0x00);
    //__disable_interrupts();
    __schedule();
    //__enable_interrupts();
}*/

void __f1_handler(void) {
    printf("[F1]");
}

void __up_handler(void) {
    // use arrows to choose the input buffer
    // cyclic array of ptrs to input buffers
}

static int32_t wake_task = -1; // TODO: use list for "tasks to wake"

__attribute__((interrupt)) static void __ps2_irq1_handler(INTERRUPT_FRAME *frame) {
    static uint32_t state = 0;

    static bool
        extended = FALSE,
        released = FALSE,
        shift = FALSE;

    uint8_t data = __inb(PS2_DATA_PORT_REGISTER);

    if (state == 0 && data == 0xe0) {
        extended = TRUE;
        ++state;
    } else if ((state == 0 || state == 1) && data == 0xf0) {
        released = TRUE;
        ++state;
    } else if (state == 0 || state == 1 || state == 2) {
        if (released) {
            // key released
            switch (data) {
                case 0x12:
                    shift = FALSE;
                    break;
            }

            extended = FALSE;
            released = FALSE;
        } else {
            // key pressed
            switch (data) {
                case 0x12:
                    shift = TRUE;
                    break;
                
                default:
                    if (data >= TABLE_SIZE) {
                        //printf("ps2: cannot translate scan code 0x%02x\n", data);
                        __send_eoi(0x01);
                        return;
                    }

                    if (extended) {
                        uint32_t fn = table_extended[data];

                        if (fn & 0xffffff00)
                            ((void (*)(void))fn)(); // call the handler
                    } else {
                        uint32_t c = shift ? table_shift[data] : table_normal[data];

                        if (c & 0xffffff00) {
                            ((void (*)(void))c)(); // call the handler
                            break;
                        }

                        // is the char printable?
                        if (!c)
                            break;

                        /*if (*/putc(c, stdin);/*)*/
                            //printk("ps2: stdin full\n"); // TODO: realloc buffer

                        // FIXME: this can cause dead-lock!!!
                        //  we should something like queue
                        putchar(c);
                        
                        /*if (c == '\n' && wake_task != -1)
                            __wake_task(wake_task);*/
                    }

                    break;
            }

            extended = FALSE;
        }

        state = 0;
    } else {
        printk("ps2: port-a: unknown scan codes sequence\n");
        printk("ps2: port-a: disabling scanning\n");

        errno = 0;
        __ps2_disable_scanning();

        // RFC: change `state' to blocked and print error message?
        if (errno) {
            printk("ps2: port-a: failed to disable scanning\n");
            printk("ps2: port-a: dettaching keyboard handler\n");

            __disable_interrupts();
            __disable_irq(1);
            __set_handler(0x01, 0x0008, INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE | INTERRUPT_DESCRIPTOR_PRESENT, &__default_isr);
            __enable_interrupts();
        }
    }

    __send_eoi(0x01);
}

__attribute__((interrupt)) static void syscall(INTERRUPT_FRAME *frame) {
    __enable_interrupts();
    __exit(0);
}

/**
 * xatoi
*/

int32_t xatoi(char const *s) {
    if (!s)
        return 0;

    while (*s == ' ') ++s; // skip whitespaces

    int32_t value = 0;

    for (char c; ((c = *s) >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); ++s) {
        c -= '0';

        if (c > 9)
            c -= 'a' - '9' - 1;

        value = 16 * value + c;
    }

    return value;
}

static void __path(struct __dentry *node) {
    if (!node->d_parent) {
        printf(node->name);
        return;
    }

    __path(node->d_parent);
    printf("/%s", node->name);
}

static void __tree(struct __dentry *node) {
    struct __dentry *child = node->d_child;

    while (child) {
        __path(child);
        putchar('\n');

        if (child->d_child)
            __tree(child);

        child = child->d_next;
    }
}

void __terminal_task(void) {
    wake_task = __get_pid();
    printk("\033[33mterminal:\033[37m terminal daemon running, PID=%u\n", __get_pid());

    struct __dentry *dhome = (struct __dentry *)kmalloc(sizeof(struct __dentry));

    if (!dhome)
        panic();

    struct __inode *ihome = (struct __inode *)kmalloc(sizeof(struct __inode));

    if (!ihome)
        panic();

    __dentry_init(dhome);
    dhome->name = "home";
    __inode_init(ihome, dhome);
    ihome->i_mode = 0x80000000 | 0777;
    dhome->d_inode = ihome;
    __dentry_add(dhome, __get_dentry());

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
            int32_t terminal_pid = __create_thread("terminal", (int32_t (*)(int argc, char **argv))&__terminal_task, THREAD_RING_0);

            if (terminal_pid == -1)
                printk("failed to start terminal\n");

            __exit(-1);
        }

        input_buffer[index] = '\0';

        char *cmd = strtok(input_buffer, " ");

        if (!strcmp(cmd, "heap")) {
            __dump_heap();
        } else if (!strcmp(cmd, "clear")) {
            __clear();
        } else if (!strcmp(cmd, "ps")) {
            __list_threads();
        } else if (!strcmp(cmd, "e820")) {
            __e820_dump_mmap();
        } else if (!strcmp(cmd, "ls")) {
            //DIR *dir = opendir(pwd);
            //__fat12_list_rootdir();
            char *s = strtok(NULL, " ");

            uint32_t mode = 0;

            if (!strcmp(s, "-l"))
                mode = 1;

            struct __dentry *child = __get_dentry()->d_child;

            if (child) {
                while (child) {
                    if ((child->d_inode->i_mode & 0777) == 0777)
                        printf("\033[30;42m");

                    if (child->d_inode->i_mode & 0x80000000)
                        printf("\033[34m");

                    printf("%s\033[37;40m", child->name);

                    switch (mode) {
                        case 0:
                            printf("  ");
                            break;

                        case 1:
                            putchar('\n');
                            break;
                    }

                    child = child->d_next;
                }
                
                if (!mode)
                    putchar('\n');
            }
        } else if (!strcmp(cmd, "hexdump")) {
            char *address = strtok(NULL, " ");

            if (!address) {
                printf("hexdump: expected address\n");
                kfree(input_buffer);
                continue;
            }
            
            char *count = strtok(NULL, " ");

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
            __tree(__get_dentry());
        else if (!strcmp(cmd, "ping")) {
            char *target = strtok(NULL, " ");

            if (!target) {
                printf("ping: expected target\n");
                kfree(input_buffer);
                continue;
            }

            //ping(target);
        } else if (index)
            printf("terminal: %s: command not found\n", cmd);

        kfree(input_buffer);
    }
}

void __user_daemon(void) {
    //printk("\033[33muser:\033[37m USER daemon running, PID=%u\n", __get_pid());

    //asm volatile ("int 0x80");
    for (;;);
}

/**
 * __init_handlers
*/

void __init_handlers(void) {
    __set_handler(0x00, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__division_by_zero);
    __set_handler(0x02, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__nmi);
    __set_handler(0x04, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__overflow);
    __set_handler(0x05, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__bounds_check);
    __set_handler(0x06, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__invalid_opcode);
    __set_handler(0x07, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__device_not_available);
    __set_handler(0x08, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__double_fault);
    __set_handler(0x0d, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__general_protection_fault);
    __set_handler(0x0e, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__page_fault);
}

static char command_line[256];
__kdev_t root_dev = NO_DEV;
char *envs[16];

/**
 * entry
*/

void entry(uint32_t e820_entries_count, E820_ENTRY *e820_entries, void *paging_directory, uint32_t cursor_y, uint32_t cursor_x, uint32_t boot_drv, uint32_t fs_type, SYMBOL *symbol_table, uint32_t symbols_count, char *string_table) {
    // reset tick counter
    __init_tick_counter();
    
    // initialize vga
    __init_vga();

    __setcurpos(cursor_y, cursor_x);

    printf("VGA initialized\n");

    // copy command line to buffer
    strcpy(command_line, (char const *)0x00007e00);

    // initialize global descriptor table
    __init_gdt(0x0010, 0);

    // fix memory map
    __e820_sanitize(e820_entries_count, e820_entries);

    // dump the map
    __e820_dump_mmap();

    asm volatile (
        "mov %0, cr3"
        : "=r" (page_directory)
        :
        :
    );
    
    page_directory &= ~31; // discards cd, wt and res

    // initialize interrupt descriptor table
    if (__init_idt(&__default_isr)) {
        printk("\033[91mFailed to initialize IDT\033[37m\n");
        panic();
    }

    // set default handlers
    // TODO: use isr wrapper
    __init_handlers();

    __parse_config(command_line);

    if (root_dev == NO_DEV) {
        // RFC: use some kind of default device?
        printk("kernel: error: unknown root dev\n");
        panic();
    }

    void *heap = __e820_rmalloc(4*4096, TRUE);

    if (!heap) {
        printk("kernel: error: failed to allocate memory for heap\n");
        panic();
    }

    // initialize physical page frame allocator
    if (__init_pager()) {
        printk("\033[91mFailed to initialize PMM\033[37m\n");
        panic();
    }

    // initialize virtual memory manager
    if (__init_vmm()) {
        printk("\033[91mFailed to initialize VMM\033[37m\n");
        panic();
    }

    // initialize kernel heap manager
    __init_heap(heap, 4*4096);

    uint32_t const __stdin_size = 128;
    char *__stdin_base = kmalloc(__stdin_size * sizeof(char));

    if (!__stdin_base) {
        printk("stdio: failed to allocate memory for stdin\n");
        panic();
    }

    *stdin = (FILE){
        .__base = __stdin_base,
        .__ptr = __stdin_base,
        .__index = 0,
        .__count = 0,
        .__flags = 0,
        .__size = __stdin_size,
        .__fname = "stdin"
    };

    // there is only one root dentry
    // because multitasking is disabled
    // at this point
    // we'll be able to set different
    // fs dentries for each process once
    // the multitasking is enabled
    struct __dentry root = {
        .d_child = NULL,
        .d_inode = NULL,
        .d_next = NULL,
        .d_parent = NULL,
        .d_prev = NULL,
        .d_refs = 1,
        .name = "" // empty string
    };

    if (__init_sysfs(&root)) {
        __dump_heap();
        panic();
    }

    if (__init_drivers()) // register "driver" group
        panic();

    /*if (__init_buses()) // register "bus" group
        panic();*/

    /**
     * initialize devices
     * 
     * supported devices:
     *  - acpi
     *  - pic(s)
     *  - pit
     *  - ps/2 (initialize usb subsystem first)
     *  - fdc
    */

    if (__init_acpi()) {
        printk("\033[91mFailed to initialize ACPI\033[37m\n");
        printk("\033[33mkernel:\033[37m \033[96mCannot use APIC => using PIC\033[37m\n");

        // initialize pic
        // initialize pic
        __init_pics(0x20, 0x70); // irqs 0-7 -> int 20->27, irqs 8-f -> 70->77
        __disable_irqs();
        __send_slave_eoi();
        __send_master_eoi();
        // set handlers for spurious interrupts
        __set_handler(0x77, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__spurious_irq_isr);
        __set_handler(0x27, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__spurious_irq_isr);
        __enable_irq(2); // enable cascade
        __enable_interrupts(); // we are ready to receive external interrupts
    } else {
        // initialize apic
        __disable_irqs(); // mask all pic irqs
    }
    
    __init_pit();

    // channel 0 for ticks counter, FIXME: doesn't work in bochs
    uint16_t const ticks = 0x001234de / 1000; // channel 0 freq=1kHz
    __outb(PIT_CHANNEL_0_DATA_REGISTER, (uint8_t)ticks);
    __outb(PIT_CHANNEL_0_DATA_REGISTER, (uint8_t)(ticks >> 8));

    __disable_interrupts();
    __set_handler(0x20, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, (void (*)(INTERRUPT_FRAME *))&__schedule);
    __enable_interrupts();
    __enable_irq(0x00); // irq0

    // TODO: load from configuration
    __set_scancode_handler(table_normal, F1_SCANCODE, (uint32_t)&__f1_handler);
    __set_scancode_handler(table_extended, 0x75, (uint32_t)&__up_handler);
    //__set_scancode_handler(table_normal, F2_SCANCODE, (uint32_t)&__f2_handler);
    //table_shift[F1_SCAN_CODE] = (uint32_t)&__f1_handler;

    // initialize usb controller first
    // ps/2 could be emulated (usb
    // legacy support)
    if (!__init_ps2()) {
        __disable_interrupts();
        __set_handler(0x21, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__ps2_irq1_handler);
        __enable_interrupts();
        __enable_irq(0x01); // irq1
    }

    // initialize first task (kernel) with default
    // root dentry (singletasking root)
    if (__sched_init(&root))
        panic();

#ifdef CONFIG_FDC
    if (__init_fdc())
        printk("kernel: info: fdc not initialized\n");
#endif

    __init_pci();

    /*printk("%p", __lookup(&root, "/sys/driver/pci", 3));
    for(;;);*/

    int32_t terminal_pid = __create_thread("terminal", (int32_t (*)(int argc, char **argv))&__terminal_task, THREAD_RING_0);

    if (terminal_pid < 0) {
        printk("failed to start terminal\n");
        panic();
    }

    //__set_handler(0x80, 0x0008, 0xee, &syscall);

    //__create_thread("user-daemon", (int32_t (*)(int argc, char **argv))&__user_daemon, 0);

    //printk("\033[33mkernel:\033[37m Entering IDLE loop\n");
    printf("\033[97mWelcome!\033[37m\n");
    printf("\033[97m%s-%s\033[37m\n", VERSION, ARCH);

    //__mount(root_dev, "dev");

    // idle loop
    for (;;)
        asm volatile ("hlt");

    /*for (uint32_t i = 0; i < symbols_count; ++i) {
        SYMBOL *symbol = &symbol_table[i];

        printf("symbol %u:\n name (", i);

        if (*((uint32_t *)symbol->symbol_name)) {
            printf("short): ");
            for (uint32_t j = 0; j < 8; ++j) putchar(symbol->symbol_name[j]);
        } else printf("long): %s", string_table + *((uint32_t *)(symbol->symbol_name + 4)));
        
        char *type;

        switch (symbol->symbol_type) {
            case 0: type = "undefined"; break;
            case 1: type = "absolute"; break;
            case 2: type = "debugging"; break;
            default: type = "unknown"; break;
        }

        printf("\n type: %s\n section number: %u\n\n", type, symbol->section_number);
        //__delay_ms(2000);
    }*/

    //__set_handler(0x80, 0x0008, 0xee, &syscall); // FIXME: implement setting dpl, this is stupid
}