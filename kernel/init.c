/**
 * @file init.c
 * @author verner002
 * @date 18/07/2026
*/

#define LOWMEM_END              (((768 + 128) << 20) - 1)

#include <stddef.h>
#include <errno.h>

#include <kernel/system.h>
#include <kernel/types.h>
#include <kernel/syscalls.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/vga.h>
#include <kernel/printk.h>
#include <kernel/io.h>
#include <kernel/kdev.h>
#include <kernel/config.h>
#include <kernel/e820.h>
#include <kernel/paging.h>
#include <kernel/pfa.h>
#include <kernel/cpu.h>
#include <kernel/82077aa.h>
#include <kernel/pci.h>
#include <kernel/ide.h>
#include <kernel/process.h>
#include <kernel/irqs.h>
#include <kernel/8254a.h>
#include <kernel/panic.h>
#include <kernel/tty.h>
#include <kernel/kmalloc.h>
#include <kernel/inode.h>

extern char __kernel_end[];

volatile uint32_t ticks;

kdev_t root_dev = NO_DEV;
char *envs[16];

volatile pid_t cproc;
volatile size_t nproc;
struct process proctab[NPROC];

struct e820_map mmap;

void *fdc_buffer;
size_t fdc_buffer_size;

__attribute__((interrupt)) static void int0_handler(struct interrupt_frame *frame) {
    printk("#de");
    panic();
}

__attribute__((interrupt)) static void int1_handler(struct interrupt_frame *frame) {
    printk("#db");
    panic();
}

__attribute__((interrupt)) static void int2_handler(struct interrupt_frame *frame) {
    printk("nmi");
    panic();
}

__attribute__((interrupt)) static void int3_handler(struct interrupt_frame *frame) {
    printk("#bp(pid=%i, parent=%i)\n", cproc, proctab[cproc].parent_pid);
    suspend(cproc);
}

__attribute__((interrupt)) static void int4_handler(struct interrupt_frame *frame) {
    printk("#of");
    panic();
}

__attribute__((interrupt)) static void int5_handler(struct interrupt_frame *frame) {
    printk("#br");
    panic();
}

__attribute__((interrupt)) static void int6_handler(struct interrupt_frame *frame) {
    printk("#ud");
    panic();
}

__attribute__((interrupt)) static void int7_handler(struct interrupt_frame *frame) {
    printk("#nm");
    panic();
}

__attribute__((interrupt)) static void int8_handler(struct interrupt_frame *frame) {
    printk("#df");
    panic();
}

__attribute__((interrupt)) static void int10_handler(struct interrupt_frame *frame) {
    printk("#ts");
    panic();
}

__attribute__((interrupt)) static void int11_handler(struct interrupt_frame *frame) {
    printk("#np");
    panic();
}

__attribute__((interrupt)) static void int12_handler(struct interrupt_frame *frame) {
    printk("#ss");
    panic();
}

__attribute__((interrupt)) static void int13_handler(struct interrupt_frame *frame) {
    printk("#gp at %p", frame->eip);
    panic();
}

__attribute__((interrupt)) static void int14_handler(struct interrupt_frame *frame, uint32_t error_code) {
    uint32_t cr2;

    asm volatile (
        "mov %0, cr2"
        : "=r" (cr2)
        :
        :
    );
    
    printk("#pf at %p with %p\n", cr2, error_code);
    panic();
}

__attribute__((interrupt)) static void int16_handler(struct interrupt_frame *frame) {
    printk("#mf");
    panic();
}

__attribute__((interrupt)) static void int17_handler(struct interrupt_frame *frame) {
    printk("#ac");
    panic();
}

__attribute__((interrupt)) static void int18_handler(struct interrupt_frame *frame) {
    printk("#mc");
    panic();
}

__attribute__((interrupt)) static void int19_handler(struct interrupt_frame *frame) {
    printk("#xm");
    panic();
}

__attribute__((interrupt)) static void int20_handler(struct interrupt_frame *frame) {
    printk("#ve");
    panic();
}

__attribute__((interrupt)) static void int21_handler(struct interrupt_frame *frame) {
    printk("#cp");
    panic();
}

__attribute__((interrupt)) static void spurious_interrupt_handler(struct interrupt_frame *) {
    
}

extern void con_init(void);
extern void con_write(struct tty_device *tty);

int io_ok(void) {
    return 0;
}

int io_err(void) {
    return -1;
}

struct device {
    char *name;
    int major;
    int minor;
    int (*open)(struct device *device, char const *name, char const *mode);
    int (*write)(struct device *device, void *buffer, size_t count);
} devtab[256] = {
    { // device 0
        "null",
        0, 0,
        (int (*)(struct device *device, char const *name, char const *mode))&io_ok,
        (int (*)(struct device *device, void *buffer, size_t count))&io_ok
    },
    { // device 1
        "tty0",
        4, 0,
        (int (*)(struct device *device, char const *name, char const *mode))&io_ok,
        (int (*)(struct device *device, void *buffer, size_t count))&io_err
    }
};

int sys_open(struct regs *r) {
    return -ENOSYS;
}

int sys_read(struct regs *r) {
    return -ENOSYS;
}

int sys_write(struct regs *r) {
    int fd = (int)r->ebx;
    void *buffer = (void *)r->ecx;
    size_t count = (size_t)r->edx;

    struct file *f;

    if (fd >= NFDESC || !(f = proctab[cproc].fdescs[fd]))
        return -ENOENT;

    struct inode *i = f->dentry->inode;
    return i->fops->write(i, f, buffer, count);
}

int sys_close(struct regs *r) {
    return -ENOSYS;
}

int (*syscall_table[])(void) = {
    (int (*)(void))&sys_fork,
    (int (*)(void))&sys_open,
    (int (*)(void))&sys_read,
    (int (*)(void))&sys_write,
    (int (*)(void))&sys_close
};

size_t const NSYSCALL = sizeof(syscall_table) / sizeof(int (*)(void));

__attribute__((interrupt)) void int32_handler(struct interrupt_frame *);
__attribute__((interrupt)) void int128_handler(struct interrupt_frame *);

void do_timer(uint32_t cpl) {
    ++ticks;

    if (!cpl) {
        ++proctab[cproc].s_time;
        return;
    }

    ++proctab[cproc].u_time;
    schedule();
}

#define STDOUT      0
#define STDIN       1
#define STDERR      2

extern int tty_write(struct inode *inode, struct file *file, void const *buffer, size_t count);

extern void enter_userland();

void init(void) {
    //write(STDOUT, "ahoj!\n", 6);
}

extern int uart_init(void);

void main(int cursor_x, int cursor_y, char *config, size_t e820_nentries, struct e820_entry *e820_entries) {
    // reinitialize gdt
    gdt_init();

    // initialize idt
    idt_init();

    install_interrupt_gate(0, &int0_handler);
    install_interrupt_gate(1, &int1_handler);
    install_interrupt_gate(2, &int2_handler);
    install_trap_gate(3, &int3_handler); // irqs must remain enabled
    install_trap_gate(4, &int4_handler);
    install_interrupt_gate(5, &int5_handler);
    install_interrupt_gate(6, &int6_handler);
    install_interrupt_gate(7, &int7_handler);
    install_interrupt_gate(8, &int8_handler);
    install_interrupt_gate(10, &int10_handler);
    install_interrupt_gate(11, &int11_handler);
    install_interrupt_gate(12, &int12_handler);
    install_interrupt_gate(13, &int13_handler);
    install_interrupt_gate(14, (void (*)(struct interrupt_frame *))&int14_handler);
    install_interrupt_gate(16, &int16_handler);
    install_interrupt_gate(17, &int17_handler);
    install_interrupt_gate(18, &int18_handler);
    install_interrupt_gate(19, &int19_handler);
    install_interrupt_gate(20, &int20_handler);
    install_interrupt_gate(21, &int21_handler);

    uint32_t cr3;
    uint32_t *pd;

    asm volatile (
        "mov %0, cr3"
        : "=r" (cr3)
        :
        :
    );

    pd = (uint32_t *)(cr3 & ~0xfff);

    vga_init();
    vga_set_cursor_pos(cursor_x, cursor_y);

    parse_config(config);

    // reset tick counter
    ticks = 0;

    // initialize process table entries
    for (size_t i = 1; i < NPROC; ++i)
        proctab[i] = (struct process){
            0,
            PROC_FREE,
            0,
            0, 0,
            NULL,
            0,
            0, 0, 0,
            0, 0, 0,
            -1,
            { NULL, NULL },
            { 0 }
        };

    proctab[0] = (struct process){
        0,                              // pid
        PROC_RUNNING,                   // state
        0,                              // priority
        0, 0,                           // system time, user time
        NULL,                           // current working directory
        cr3,                            // cr3
        0, 0, 0,                        // stack
        0, 0, 0,                        // kernel stack
        -1,                             // parent
        { NULL, NULL },
        { 0 }
    };

    nproc = 1;
    cproc = 0;

    // sanitize memory map
    e820_sanitize(e820_nentries, e820_entries);

    // discard direct mapping
    pd[0] = 0;

    //e820_dump_mmap();

    struct e820_entry *ldesc = &mmap.entries[mmap.nentries - 1];

    uint32_t ram_end = ldesc->base + ldesc->size - 1;

    uint32_t lowmem_end = ram_end < LOWMEM_END ? ram_end : LOWMEM_END;

    uint32_t kernel_end_addr = (uint32_t)&__kernel_end;
    //bump_alloc = (kernel_end_addr + 4095) & ~4095;

    // up to 4 MiB of memory is identity- and
    // 0xc0000000- mapped at this point (that
    // is minimal expected memory size), now
    // we have to map the remaining LOWMEM -
    // memory instantly accessible to kernel

    // first pde is filled (first pt is ready)
    // we have to allocate memory page tables
    // inside this region and then map the
    // remaining 768+128-4 MiB

    size_t const npt = ((((size_t)lowmem_end + 1) / (1 << 20)) - 4) / 4;
    size_t const npt_bytes = npt * 4096;

    uint32_t *pte = e820_alloc(npt_bytes, 1, 4 << 20) + KERNEL_VIRT_BASE;

    printk("npt=%u (%u bytes)\npt_array=%p\n", npt, npt_bytes, pte);

    size_t npte = npt * 1024;

    // TODO: do not perform mapping of last 128 MiB
    for (size_t i = 0; i < npte; ++i)
        pte[i] = ((4 << 20) + (i * (4 << 10))) | PAGE_CD | PAGE_WT | PAGE_RW | PAGE_P;

    for (size_t i = 0; i < npt; ++i)
        pd[i + 768 + 1] = ((uint32_t)pte + i * (4 << 10)) | PAGE_CD | PAGE_WT | PAGE_RW | PAGE_P;

    // up to 768 MiB of memory is now identity mapped
    // and directly accessible by kernel (last 128 MiB)
    // can be used for dynamic (temporary) mapping

#ifdef CONFIG_FDC
    // dma buffer must reside in the first MiB
    // of physical memory
    fdc_buffer = e820_alloc(fdc_buffer_size = 512, 1, 1 << 20);

    // TODO: initialize fdc in non-dma mode
    if (!fdc_buffer) {
        fdc_buffer_size = 0;
        printk("kernel: fdc: buffer alloc failed\n");
        printk("kernel: fdc init will be skipped\n");
    }
#else
    fdc_buffer_size = 0;
    fdc_buffer = NULL;
#endif

    pfa_init(&mmap);

    void *heap_start = (void *)((uint32_t)pfa_alloc() + KERNEL_VIRT_BASE);

    kheap_init(heap_start, 4096);

    printk("heap_start=%p, heap_end=%p\n", heap_start, heap_start + 4096 - 1);

    irqs_init();

    // enable cascade irq
    enable_irq(0x02);

    // install spurious irqs handlers
    install_interrupt_gate(0x27, &spurious_interrupt_handler);
    install_interrupt_gate(0x77, &spurious_interrupt_handler);

    // enable external interrupts
    sti();

    // initialize uart
    int uart = uart_init();
    printk("kernel: uart: init: %i\n", uart);

    // initialize pit
    pit_init();

    // initialize pci bus and pci devices
    //pci_init();

    // channel 0 freq=1kHz
    uint16_t const t = 0x001234de / 1000;
    outb(0x40, (uint8_t)t);
    outb(0x40, (uint8_t)(t >> 8));

    install_interrupt_gate(0x20, &int32_handler);

    // enable pit irq
    enable_irq(0);

    // set user stack
    uint32_t *stack_phys = pfa_alloc();
    uint32_t *stack = (uint32_t *)(KERNEL_VIRT_BASE - 4096);
    
    int map = kmap((void *)(cr3 & ~4095), (void *)stack, (void *)stack_phys, PAGE_CD | PAGE_WT | PAGE_US | PAGE_RW | PAGE_P);

    if (map) {
        printk("kmap failed\n");
        for (;;);
    } else
        printk("kmap ok\n");

    proctab[0].stack_base = (unsigned long)stack;
    proctab[0].stack_size = 4096;

    stack += 1024;
    *--stack = STACK_CANARY;
    proctab[0].stack_ptr = (unsigned long)stack;

    asm volatile (
        "mov %0, esp"
        : "=r" (proctab[0].kstack_ptr)
        :
        :
    );

    asm volatile (
        "mov esp, %0"
        :
        : "r" (stack)
        : "memory"
    );

    /*uint32_t *kernel_kstack = (uint32_t *)((uint32_t)pfa_alloc() + KERNEL_VIRT_BASE);

    proctab[0].stack_base = (unsigned long)kernel_kstack;
    proctab[0].stack_size = 4096;

    // kernel stack setup
    kernel_kstack += 1024;
    *--kernel_kstack = STACK_CANARY;
    proctab[0].kstack_ptr = (unsigned long)kernel_kstack;*/

    tss_set_esp0((uint32_t)proctab[0].kstack_ptr);

    // expose syscall handler
    install_user_interrupt_gate(0x80, &int128_handler);

    struct inode *root = iget(0, 0, S_IFDIR | 0755);

    struct file_ops stdout_fops = {
        .write = &tty_write
    };

    struct inode *stdout = iget(0, 0, S_IFCHR | 0700);
    stdout->fops = &stdout_fops;

    struct dentry dstdout = {
        .inode = stdout
    };

    struct file fstdout = {
        .dentry = &dstdout
    };

    proctab[0].fdescs[0] = &fstdout;

    printk("kernel: starting init\n");

    enter_userland();

    if (!fork())
        init();

    // become idle
    for (;;)
        asm volatile ("pause" :::);

    // try to initialize ps2
    // NOTE: we won't do that here (parse ACPI first)
    /*if (!ps2_init()) {
        // install keyboard irq handler
        //idt_install_handler(0x21, &keyboard_irq_handler);
        
        // enable keyboard irq
        pic_enable_irq(0x01);
    }*/

    /*asm volatile (
        "mov %0, cr3"
        : "=r" (cr3)
        :
        :
    );*/

    // TODO: initialize fdc

    // at this point we have to export architecture specific functions
    // to the kernel so it is able to use hardware the same way on almost
    // every platform
    // after the vfs initialization, this hardware layer should be able
    // to create device structure in the vfs so user can interact with it
    
    // TODO: init scheduler
    // TODO: start ksoftirq daemon (better do in kernel)
    // TODO: install handlers (ksoftirq)

    // export e820_alloc
    //  - kernel will use it to allocate memory
    //     for kheap
    //
    // export vga_write
    //  - kernel will use it in tty0
}