/**
 * @file main.c
 * @author verner002
 * @date 11/10/2024
 * @note j+k=<3
*/

//#define CONFIG_FDC
#define COMMAND_LINE 0x00080200

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

#include "fs/file.h"

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

#include "kernel/panic.h"

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
    uint32_t vas;

    asm volatile (
        "mov eax, cr2"
        : "=a" (vas)
        :
        :
    );

    if (thread_current && __get_pid() != 0) {
        __enable_interrupts();
        printf("Segmentation fault\n");
        __exit(-1);
    }

    printk("page-fault when accessing address %p\n", vas);
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
        extended = false,
        released = false,
        shift = false;

    uint8_t data = __inb(PS2_DATA_PORT_REGISTER);

    if (state == 0 && data == 0xe0) {
        extended = true;
        ++state;
    } else if ((state == 0 || state == 1) && data == 0xf0) {
        released = true;
        ++state;
    } else if (state == 0 || state == 1 || state == 2) {
        if (released) {
            // key released
            switch (data) {
                case 0x12:
                    shift = false;
                    break;
            }

            extended = false;
            released = false;
        } else {
            // key pressed
            switch (data) {
                case 0x12:
                    shift = true;
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

                        if (c == '\b' && stdin->__count || c != '\b') {
                            __putc(c); //__graphix_putc(c);
                            //__vbe_redraw();
                        }

                        if (stdin)
                            putc(c, stdin);

                        /*if (putc(c, stdin);)*/
                            //printk("ps2: stdin full\n"); // TODO: realloc buffer

                        // FIXME: this can cause dead-lock!!!
                        //  we should something like queue

                        //putc(c, (FILE *)0xdeadbabe);
                        
                        /*if (c == '\n' && wake_task != -1)
                            __wake_task(wake_task);*/
                    }

                    break;
            }

            extended = false;
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

struct dentry *dentry_lookup(struct dentry *node, char const *path) {
    static bool init = true;

    if (!path)
        return NULL;
    
    char *name = strtok(init ? path : NULL, "/");
    init = false;

    // we've reached the end of the path
    if (!name) {
        init = true;
        return node;
    }

    struct dentry *child = node->inode->child; // children list head
    
    while (child) {
        if (!strcmp(child->name, name))
            return dentry_lookup(child, name);
        
        child = child->next;
    }

    init = true;
    return NULL;
}

// use atomic operations
bool net_rx_pending = false;

void receive_packet() {
    printf("processing packet...\n");
    net_rx_pending = false;
}

bool deffered_job = false;

void __softirq_daemon(void) {
    __wake_on(&deffered_job);

    for (;;) {
        if (net_rx_pending) {
            receive_packet();
            //net_rx_pending = false;
        }
    }

    deffered_job = false;
}

bool pci_exited = false;
int pci_exit_code;

void pci_exit_handler(int exit_code) {
    pci_exit_code = exit_code;
    pci_exited = true;
}

bool fdc_exited = false;
int fdc_exit_code;

void fdc_exit_handler(int exit_code) {
    fdc_exit_code = exit_code;
    fdc_exited = true;
}

/**
 * main
*/

void main(void) {
    // TODO: create system information block structure
    uint32_t e820_entries_count = *(uint32_t *)0x90003;
    E820_ENTRY *e820_entries = (E820_ENTRY *)0x94000;
    uint32_t cursor_x = *(uint8_t *)0x90001;
    uint32_t cursor_y = *(uint8_t *)0x90002;
    uint32_t bytes_per_char = *(uint32_t *)0x90007;
    uint32_t character_rows = *(uint8_t *)0x90009;
    uint32_t font_address = *(uint32_t *)0x9000a;
    
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

    // at this point up to the first 4 MiBs of memory is
    // directly mapped and accessible to kernel, now we
    // have to map the remaining 764 MiBs (if present in
    // the system) and reserve memory for dynamic mapping
    // of the last 256 MiBs, 1 MiB is required to map all
    // the page tables (for 768 MiB + 256 MiB kernel mapping)
    //
    // so we tell e820 allocator to reserve 1 MiB block and
    // use memory from up to the first 4 MiB (directly mapped)
    // for that
    //
    // RFC: do we always want to perform this? what if there
    //  is less than 768 MiB in the system?
    PAGE_TABLE_ENTRY *page_table_entries = (PAGE_TABLE_ENTRY *)e820_alloc(1*1024*1024, true, 4*1024*1024);

    if (!page_table_entries)
        panic();

    for (uint32_t i = 4*1024*1024/4096; i < 768*1024*1024/4096; ++i) {
        page_table_entries[i] = (PAGE_TABLE_ENTRY){
            .address = i*4096,
            .present = 1,
            .cache_disabled = 1,
            .read_write = 1,
            .user_supervisor = 1,
            .write_through = 1,
            .page_attribute_table = 0
        };
    }

    for (uint32_t i = 1; i < 768*1024*1024/4096/1024; ++i) {
        void *page_table = (void *)page_table_entries + i*(sizeof(PAGE_TABLE_ENTRY[1024]));
        PAGE_DIRECTORY_ENTRY *pde = &((PAGE_DIRECTORY_ENTRY *)page_directory)[i];
        pde->address = (uint32_t)page_table >> 12;
        pde->granularity = 0;
        pde->read_write = 1;
        pde->user_supervisor = 1;
        pde->cache_disabled = 1;
        pde->write_through = 1;
        pde->present = 1;
    }

    // map up to 768 MiB of ram statically and remaining
    // 256 MiB dynamically (3g/1g scheme)
    // allocate page for a page table that will be used
    // to map the 1 MiB block for 3g/1g scheme mapping
    // page tables

    // allocate 1 MiB of physical memory for page tables
    // memory for static and dynamic mappingsizeof(PAGE_TABLE_ENTRY) * 
    // 768 mb for static, 256 mb for dynamic
    // 768 mb will be always immediately available
    // for kernel to use, 256 mb will be used for DMA,
    // on-demand mapping, etc.

    // each process virtual address space will contain 1g kernel
    // mapped virtual address space (its paging directory will
    // have reference to kernel page tables), since more virtual
    // addresses can refer to the same physical address, kernel
    // can perform allocations from its mapped physical address
    // space and add an mapping to lower 3g virtual address space
    //
    // basically up to 768 MiB will be always statically mapped by
    // kernel but also usable by user space if needed (if kernel
    // performs mapping to given virtual address space)

    // TODO: map up to 768 MiB of memory to 0xc0000000 line
    // this allow use to always access some physical memory
    // without need to perform mapping (dependend od memory
    // allocation itself), remaininf 256 MiB will be used
    // as dynamically mapped memory in case we run out of
    // statically mapped memory
    //
    // NOTE: let the kernel start at address 0? but we use executable
    // format for kernel image, that could be a problem

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
    uint32_t const heap_size = 128*1024;

    void *heap = e820_alloc(heap_size, true, 768*1024*1024);

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

    //__graphix_init((void *)0x90400, (VBE_MODE_INFO *)0x90600, (char const *)font_address, bytes_per_char);

    // initialize kernel heap manager
    __init_heap(heap, heap_size);

    // initialize graphix (vbe)
    // initialize vga again? (already initialized by default)
    //__graphix_init(NULL, (VBE_MODE_INFO *)0x90600, (char const *)font_address, bytes_per_char, false);

    // there is only one root dentry
    // because multitasking is disabled
    // at this point
    // we'll be able to set different
    // fs dentries for each process once
    // the multitasking is enabled

    struct inode root_inode = {
        .uid = 0,
        .gid = 0,
        .mode = 0x80000000 | 0755,
        .child = NULL,
        .refs = 1,
        .i_ops = NULL,
        .size = 0,
        .super_block = NULL,
        .fs_data = NULL
    };

    struct dentry_ops root_ops = {
        .lookup = &dentry_lookup
    };

    struct dentry root_dentry = {
        .parent = NULL,
        .name = "", // change to "/"
        .previous = NULL,
        .next = NULL,
        .d_ops  = &root_ops,
        .refs = 1,
        .inode = &root_inode
    };

    if (sysfs_init(&root_dentry)) {
        printk("kernel: error: failed to initialize sysfs\n");
        panic();
    }

    if (drivers_init()) { // register "driver" group
        printk("kernel: error: failed to initialize drivers\n");
        panic();
    }

    struct dentry *mnt = create_file(&root_dentry, "mnt", 0, 0, 0x80000000 | 0755);
    struct dentry *tmp = create_file(&root_dentry, "tmp", 0, 0, 0x800001ff);
    struct dentry *home = create_file(&root_dentry, "root", 0, 0, 0x80000000 | 0750);
    home->inode->size = 4096;

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

    if (__sched_init(&root_dentry)) {
        printk("failed to start scheduler\n");
        panic();
    }

    __tty_init();

    if (__dev_init()) {
        printk("failed to start devman\n");
        panic();
    }

    // takes care of deffered jobs
    int32_t softirq_pid = 0;//__create_thread("softirq-daemon", (int32_t (*)(int argc, char **argv))&__softirq_daemon, THREAD_RING_0, THREAD_PRIORITY_HIGH);

    if (softirq_pid < 0) {
        printk("failed to start softirq\n");
        panic();
    }

#ifdef CONFIG_FDC
    int32_t fdc_init_pid = __create_thread("fdc-init", (int32_t (*)(int argc, char **argv))&__init_fdc, THREAD_RING_0, THREAD_PRIORITY_HIGH, &fdc_exit_handler);

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
    } while (!fdc_exited);

    // TODO: clear line (with spaces)
    printf("\r[");

    if (fdc_exit_code)
        printf("\033[91mFAILED");
    else
        printf("\033[92m  OK  ");

    printf("\033[37m] FDC Initialized\n");
#endif
    
    // initialize pci bus and devices
    int pci_daemon_pid = __create_thread("pci-daemon", (int (*)(int argc, char **argv))&__pci_init, THREAD_RING_0, THREAD_PRIORITY_HIGH, &pci_exit_handler);

    if (pci_daemon_pid < 0) {
        printk("failed to start pci daemon\n");
        panic();
    }

    // wait for pci daemon to initialize pci bus
    while (!pci_exited);

    // FIXME: messages can come out of order!
    //  since buffers are read in a loop by tty
    //  daemon (printing is asynchronous)
    // SOLUTION: it would be better to use global
    //  queue for stdout (not file, QUEUE!)

    if (pci_exit_code)
        printf("PCI initialization failed\n");

    /*printk("%p", __lookup(&root, "/sys/driver/pci", 3));
    for(;;);*/

    int32_t terminal_pid = __create_thread("terminal", (int32_t (*)(int argc, char **argv))&__terminal_task, THREAD_RING_0, THREAD_PRIORITY_HIGH, NULL);

    if (terminal_pid < 0) {
        printk("failed to start terminal\n");
        panic();
    }

    //__idt_set_handler(0x80, 0x0008, 0xee, &syscall);

    //printk("\033[33mkernel:\033[37m Entering IDLE loop\n");
    printf("\033[97mWelcome!\033[37m\n");
    printf("\033[97m%s-%s\033[37m\n", VERSION, ARCH);

    //__mount(root_dev, "/dev/fd0");

    // idle loop
    for (;;) {
        asm volatile ("hlt");
        __yield();
    }

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