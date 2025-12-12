/**
 * @file main.c
 * @author verner002
 * @date 11/10/2024
 * @note j+k=<3
*/

#define CONFIG_FDC
#define COMMAND_LINE 0x00090200

#ifndef VERSION
    #define VERSION "unknown"
#endif

#ifndef ARCH
    #define ARCH "unknown"
#endif

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
#include "hal/dev.h"
#include "kernel/config.h"

#include "kernel/tty.h"
#include "kernel/mount.h"
#include "kernel/terminal.h"

#include "drivers/graphics/graphix.h"

#include "kernel/tty.h"

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

__attribute__((interrupt)) static void __spurious_irq_isr(struct __interrupt_frame *frame) {
    
}

__attribute__((interrupt)) static void __default_isr(struct __interrupt_frame *frame) {
    printk("warning: unhandled interrupt!\n");
    for (;;);
}

__attribute__((interrupt)) static void __division_by_zero(struct __interrupt_frame *frame) {
    printk("fault: division by zero!\n");
    for (;;);
}

__attribute__((interrupt)) static void __nmi(struct __interrupt_frame *frame) {
    printk("info: nmi!\n");
    for (;;);
}

__attribute__((interrupt)) static void __overflow(struct __interrupt_frame *frame) {
    printk("trap: overflow!\n");
    for (;;);
}

__attribute__((interrupt)) static void __bounds_check(struct __interrupt_frame *frame) {
    printk("fault: bounds check!\n");
    for (;;);
}

__attribute__((interrupt)) static void __invalid_opcode(struct __interrupt_frame *frame) {
    uint32_t eip; // instruction that caused the fault
    
    asm volatile (
        "pop %0"
        : "=m" (eip)
        :
        :
    );

    printk("fault: invalid opcode at %p!\n", eip);

    if (thread_current)
        printk("thread %u\n", __get_pid());

    for (;;);
}

__attribute__((interrupt)) static void __device_not_available(struct __interrupt_frame *frame) {
    printk("fault: device not available!\n");
    for (;;);
}

__attribute__((interrupt)) static void __double_fault(struct __interrupt_frame *frame) {
    printk("abort: double fault!\n");
    for (;;);
}

__attribute__((interrupt)) static void __general_protection_fault(struct __interrupt_frame *frame) {
    if (thread_current && __get_pid() != 0) {
        __enable_interrupts();
        __exit(1); // no return
    }
    
    printk("fault: general protection fault!\n");
    for (;;);
}

__attribute__((interrupt)) static void __page_fault(struct __interrupt_frame *frame) {
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

/*__attribute__((interrupt)) void __pit_irq0_handler(struct __interrupt_frame *frame) {
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

__attribute__((interrupt)) static void __ps2_irq1_handler(struct __interrupt_frame *frame) {
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

                        __putc(c);
                        //putc(c, (FILE *)0xdeadbabe);
                        
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
            __idt_set_handler(0x01, 0x0008, IDT_32BIT_INTERRUPT_ENTRY | IDT_ENTRY_PRESENT, &__default_isr);
            __enable_interrupts();
        }
    }

    __send_eoi(0x01);
}

__attribute__((interrupt)) static void syscall(struct __interrupt_frame *frame) {
    __enable_interrupts();
    __exit(0);
}

/**
 * __init_handlers
*/

void __init_handlers(void) {
    __idt_set_handler(0x00, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__division_by_zero);
    __idt_set_handler(0x02, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__nmi);
    __idt_set_handler(0x04, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__overflow);
    __idt_set_handler(0x05, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__bounds_check);
    __idt_set_handler(0x06, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__invalid_opcode);
    __idt_set_handler(0x07, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__device_not_available);
    __idt_set_handler(0x08, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__double_fault);
    __idt_set_handler(0x0d, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__general_protection_fault);
    __idt_set_handler(0x0e, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__page_fault);
}

__kdev_t root_dev = NO_DEV;
char *envs[16];

struct __dentry *__dentry_lookup(struct __dentry *node, char const *path) {
    static bool init = TRUE;

    if (!path)
        return NULL;
    
    char *name = strtok(init ? path : NULL, "/");
    init = FALSE;

    // we've reached the end of the path
    if (!name) {
        init = TRUE;
        return node;
    }

    struct __dentry *child = node->d_child; // children list head
    
    while (child) {
        if (!strcmp(child->name, name))
            return __dentry_lookup(child, name);
        
        child = child->d_next;
    }

    init = TRUE;
    return NULL;
}

// use atomic operations
bool net_rx_pending = FALSE;

void receive_packet() {
    printf("processing packet...\n");
    net_rx_pending = FALSE;
}

bool deffered_job = FALSE;

void __softirq_daemon(void) {
    __wake_on(&deffered_job);

    for (;;) {
        if (net_rx_pending) {
            receive_packet();
            //net_rx_pending = FALSE;
        }
    }

    deffered_job = FALSE;
}

/**
 * entry
 * 
 * stack is at 0x0009fffe-(return+args)
*/

void entry(uint32_t e820_entries_count, E820_ENTRY *e820_entries, uint32_t cursor_x, uint32_t cursor_y, uint32_t boot_drv, uint32_t bytes_per_char, uint32_t  character_rows, uint32_t font_address) {
    // initialize gdt
    __gdt_init(0x0010);

    // initialize idt
    __idt_init();

    // install default handlers
    __init_handlers();
    
    // reset tick counter
    __init_tick_counter();
    
    // initialize vga
    __vga_init();
    __vga_set_cursor_position(cursor_y, cursor_x);

    printf("VGA initialized\n");

    // fix memory map
    __e820_sanitize(e820_entries_count, e820_entries);

    // dump memory map
    __e820_dump_mmap();

    asm volatile (
        "mov %0, cr3"
        : "=r" (page_directory)
        :
        :
    );
    
    // keep only the address
    page_directory &= ~31;

    if (__parse_config((char const *)COMMAND_LINE))
        printk("kernel: warning: failed to parse command line\n");

    if (root_dev == NO_DEV) {
        // RFC: use some kind of default root device
        //  or ramdisk?
        printk("kernel: error: unknown root dev\n");
        panic();
    }

    // allocate page-aligned memory
    // for kernel heap
    void *heap = __e820_rmalloc(16*1024, TRUE);

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
    __init_heap(heap, 16*1024);

    // initialize graphix (vbe)
    // initialize vga again? (already initialized by default)
    //__graphix_init(NULL, (VBE_MODE_INFO *)0x90600, (char const *)font_address, bytes_per_char, FALSE);

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
        .name = "", // empty string
        .io_ops = {
            .create = NULL,
            .lookup = &__dentry_lookup
        }
    };

    if (__sysfs_init(&root)) {
        printk("kernel: error: failed to initialize sysfs\n");
        panic();
    }

    if (__init_drivers()) { // register "driver" group
        printk("kernel: error: failed to initialize drivers\n");
        panic();
    }

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

    if (__acpi_init()) {
        printk("\033[91mFailed to initialize ACPI\033[37m\n");
        printk("\033[33mkernel:\033[37m \033[96mCannot use APIC => using PIC\033[37m\n");

        // initialize pic
        __pic_init(0x20, 0x70); // irqs 0-7 -> int 20->27, irqs 8-f -> 70->77
        __disable_irqs();
        __send_slave_eoi();
        __send_master_eoi();
        // set handlers for spurious interrupts
        __idt_set_handler(0x77, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__spurious_irq_isr);
        __idt_set_handler(0x27, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__spurious_irq_isr);
        __enable_irq(0x02); // enable cascade
        __enable_interrupts(); // we are ready to receive external interrupts
    } else {
        // initialize apic
        __disable_irqs(); // mask all pic irqs
    }
    
    __pit_init();

    // channel 0 for ticks counter
    __disable_interrupts();
    uint16_t const ticks = 0x001234de / 1000; // channel 0 freq=1kHz
    __outb(PIT_CHANNEL_0_DATA_REGISTER, (uint8_t)ticks);
    __outb(PIT_CHANNEL_0_DATA_REGISTER, (uint8_t)(ticks >> 8));

    __idt_set_handler(0x20, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, (void (*)(struct __interrupt_frame *))&__schedule);
    __enable_interrupts();
    __enable_irq(0x00); // irq0
    __send_eoi(0x00);

    // TODO: load from configuration
    __set_scancode_handler(table_normal, F1_SCANCODE, (uint32_t)&__f1_handler);
    __set_scancode_handler(table_extended, 0x75, (uint32_t)&__up_handler);
    //__set_scancode_handler(table_normal, F2_SCANCODE, (uint32_t)&__f2_handler);
    //table_shift[F1_SCAN_CODE] = (uint32_t)&__f1_handler;

    // initialize usb controller first
    // ps/2 could be emulated (usb
    // legacy support)
    if (!__ps2_init()) {
        __disable_interrupts();
        __idt_set_handler(0x21, 0x0008, IDT_ENTRY_PRESENT | IDT_32BIT_INTERRUPT_ENTRY, &__ps2_irq1_handler);
        __enable_interrupts();
        __enable_irq(0x01); // irq1
    }

    // initialize first task (kernel) with
    // default root dentry
    if (__sched_init(&root))
        panic();

    __tty_init();

    if (__dev_init())
        panic();

    // takes care of deffered jobs
    int32_t softirq_pid = 0;//__create_thread("softirq-daemon", (int32_t (*)(int argc, char **argv))&__softirq_daemon, THREAD_RING_0, THREAD_PRIORITY_HIGH);

    if (softirq_pid < 0) {
        printk("failed to start softirq\n");
        panic();
    }

#ifdef CONFIG_FDC
    int32_t fdc_init_pid = __create_thread("fdc-init", (int32_t (*)(int argc, char **argv))&__init_fdc, THREAD_RING_0, THREAD_PRIORITY_HIGH);

    if (fdc_init_pid < 0) {
        printk("failed to start fdc daemon\n");
        panic();
    }

    uint32_t state;
    uint32_t spaces = 0;
    int32_t add = 1;
    int tks = 0;

    // this is actually the sexiest thing in this
    // kernel right now ... absolutely love it
    do {
        if (__get_state(fdc_init_pid, &state))
            break;
        
        if (tks % 20 == 0) {
            printf("\r[");

            for (uint32_t j = 0; j < spaces; ++j)
                putchar(' ');
            
            printf("\033[33m***\033[37m");

            for (uint32_t j = 0; j < 3 - spaces; ++j)
                putchar(' ');

            printf("] Initializing... ");

            if (add == 1)
                ++spaces;
            else if (add == -1)
                --spaces;

            if (spaces <= 0 || spaces >= 3)
                add *= -1;
        }

        __delay_ms(1);
        ++tks;
    } while (state != 3);

    int32_t exitcode;

    if (__get_exitcode(fdc_init_pid, &exitcode))
        panic();

    // TODO: clear line (with spaces)
    printf("\r[");

    if (exitcode)
        printf("\033[91mFAILED");
    else
        printf("\033[92m  OK  ");

    printf("\033[37m] FDC Initialized\n");
#endif
    
    // initialize pci bus and devices
    __pci_init();

    /*printk("%p", __lookup(&root, "/sys/driver/pci", 3));
    for(;;);*/

    int32_t terminal_pid = __create_thread("terminal", (int32_t (*)(int argc, char **argv))&__terminal_task, THREAD_RING_0, THREAD_PRIORITY_HIGH);

    if (terminal_pid < 0) {
        printk("failed to start terminal\n");
        panic();
    }

    //__idt_set_handler(0x80, 0x0008, 0xee, &syscall);

    //printk("\033[33mkernel:\033[37m Entering IDLE loop\n");
    printf("\033[97mWelcome!\033[37m\n");
    printf("\033[97m%s-%s\033[37m\n", VERSION, ARCH);

    __mount(root_dev, "/dev/fd0");

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

    //__idt_set_handler(0x80, 0x0008, 0xee, &syscall); // FIXME: implement setting dpl, this is stupid
}