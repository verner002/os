/**
 * @file init.c
 * @author verner002
 * @date 18/07/2026
*/

#define LOWMEM_SIZE             ((768 + 128) << 20)
#define LOWMEM_MAX              (LOWMEM_SIZE - 1)

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
#include <kernel/slab.h>
#include <kernel/inode.h>
#include <kernel/devfs.h>
#include <kernel/vfs.h>
#include <kernel/fcntl.h>

extern char __kernel_end[];

volatile uint32_t ticks;

kdev_t root_dev = NO_DEV;
char *envs[16];

int errno;

volatile pid_t cproc;
struct process *volatile current;
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
    printk("#bp(pid=%i, parent=%i)\n", current->pid, current->parent_pid);
    suspend(current->pid);
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

int io_ok(void) {
    return 0;
}

int io_err(void) {
    return -1;
}

/*struct device {
    char *name;
    int major;
    int minor;
    int (*open)(struct device *device, char const *name, int flags);
    int (*read)(struct device *device, void *buffer, size_t count);
    int (*write)(struct device *device, void const *buffer, size_t count);
} devtab[256] = {
    { // device 0
        "null",
        0, 0,
        (int (*)(struct device *device, char const *name, int flags))&io_ok,
        (int (*)(struct device *device, void *buffer, size_t count))&io_ok,
        (int (*)(struct device *device, void const *buffer, size_t count))&io_err
    },
    { // device 1
        "tty0",
        4, 0,
        (int (*)(struct device *device, char const *name, int flags))&io_ok,
        (int (*)(struct device *device, void *buffer, size_t count))&io_err,
        
    }
};*/

int find_free_fd(void) {
    int fd;

    for (fd = 0; fd < NFDESC; ++fd)
        if (!current->fdescs[fd])
            break;

    if (fd >= NFDESC)
        return -1;

    return fd;
}

int sys_open(struct regs *r) {
    char const *path = (char const *)r->ebx;
    int flags = (int)r->ecx;
    int mode = (int)r->edx;

    int fd = find_free_fd();

    if (fd < 0)
        return -EMFILE;

    struct file *file = vfs_open(path, flags, mode);

    if (!file)
        return -ENOENT;

    current->fdescs[fd] = file;
    return fd;
}

int sys_read(struct regs *r) {
    int fd = (int)r->ebx;
    void *buffer = (void *)r->ecx;
    size_t count = (size_t)r->edx;

    struct file *f;

    if (fd < 0 || fd >= NFDESC || !(f = current->fdescs[fd]))
        return -EBADF;

    return f->fops->read(f, buffer, count);
}

int sys_write(struct regs *r) {
    int fd = (int)r->ebx;
    void *buffer = (void *)r->ecx;
    size_t count = (size_t)r->edx;

    struct file *f;

    if (fd < 0 || fd >= NFDESC || !(f = current->fdescs[fd]))
        return -EBADF;

    return f->fops->write(f, buffer, count);
}

int sys_close(struct regs *r) {
    int fd = (int)r->ebx;

    struct file *f;

    if (fd < 0 || fd >= NFDESC || !(f = current->fdescs[fd]))
        return -EBADF;
    
    int err = vfs_close(f);

    if (err)
        return err;

    current->fdescs[fd] = NULL;
    return 0;
}

int sys_dup(struct regs *r) {
    int fd = (int)r->ebx;

    struct file *f;

    if (fd < 0 || fd >= NFDESC || !(f = current->fdescs[fd]))
        return -EBADF;

    fd = find_free_fd();

    if (fd < 0)
        return -EMFILE;

    current->fdescs[fd] = f;
    ++(f->count);
    return fd;
}

int sys_dup2(struct regs *r) {
    int old_fd = (int)r->ebx;
    int new_fd = (int)r->ecx;

    struct file *old_f;

    if (old_fd < 0 || old_fd >= NFDESC || new_fd < 0 || new_fd >= NFDESC || !(old_f = current->fdescs[old_fd]))
        return -EBADF;

    if (new_fd == old_fd)
        return new_fd;

    struct file *new_f;

    if ((new_f = current->fdescs[new_fd]))
        vfs_close(new_f);

    current->fdescs[new_fd] = old_f;
    ++(old_f->count);
    return new_fd;
}

typedef int (*syscall_fn_t)(struct regs *);

syscall_fn_t syscall_table[] = {
    &sys_fork,
    &sys_open,
    &sys_read,
    &sys_write,
    &sys_close,
    &sys_dup,
    &sys_dup2
};

size_t const NSYSCALL = sizeof(syscall_table) / sizeof(syscall_fn_t);

__attribute__((interrupt)) void int32_handler(struct interrupt_frame *);
__attribute__((interrupt)) void int128_handler(struct interrupt_frame *);

void do_timer(uint32_t cpl) {
    ++ticks;

    if (!cpl) {
        ++current->s_time;
        return;
    }

    ++current->u_time;
    schedule();
}

#define STDIN       0
#define STDOUT      1
#define STDERR      2

extern int tty_write(struct inode *inode, struct file *file, void const *buffer, size_t count);

extern void enter_userland(uint32_t stack);

int number(char buf[11], int value) {
    int i = 11;
    int m = 0;

    if (value < 0) {
        value = -value;
        m = 1;
    }

    do {
        char d = (value % 10) + '0';
        buf[--i] = d;
    } while (value /= 10);

    if (m)
        buf[--i] = '-';

    return i;
}

void init(void) {
    int pid = fork();

    if (pid > 0) {
        char msg[] = "\033[33minit:\033[37m process 1\n";
        write(STDOUT, msg, sizeof(msg) - 1);
    } else if (pid == 0) {
        char msg[] = "\033[33minit:\033[37m process 2\n";
        write(STDOUT, msg, sizeof(msg) - 1);
    } else {
        char msg[] = "\033[33minit:\033[37m fork failed\n";
        write(STDERR, msg, sizeof(msg) - 1);
    }
    
    /*char buf[1024];
    int rc = read(STDIN, buf, sizeof(buf));

    char err[11];
    int off = number(err, rc);

    write(STDOUT, err + off, 11 - off);*/
}

extern struct dentry *root_dentry;

extern int e9_init(void);
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
            0, 0,
            0, 0, 0,
            -1,
            { NULL, NULL },
            { NULL }
        };

    proctab[0] = (struct process){
        0,                              // pid
        PROC_RUNNING,                   // state
        0,                              // priority
        0, 0,                           // system time, user time
        NULL,                           // current working directory
        cr3,                            // cr3
        0, 0,                           // user stack
        KERNEL_VIRT_STACK, 0, 4096,     // kernel stack
        -1,                             // parent
        { NULL, NULL },
        { NULL }
    };

    list_init(&(proctab[0].l_wait));

    for (size_t i = 0; i < NFDESC; ++i)
        proctab[0].fdescs[i] = NULL;

    nproc = 1;
    cproc = 0;
    current = proctab;

    // sanitize memory map
    e820_sanitize(e820_nentries, e820_entries);

    // discard direct mapping
    pd[0] = 0;

    e820_dump_mmap();

    uint32_t ram_end;

    for (size_t i = mmap.nentries; i > 0; --i) {
        struct e820_entry *entry = mmap.entries + i - 1;

        if (entry->type == 1) {
            ram_end = entry->base + entry->size - 1;
            break;
        }
    }

    uint32_t lowmem_end = ram_end < LOWMEM_MAX ? ram_end : LOWMEM_MAX;

    //uint32_t kernel_end_addr = (uint32_t)&__kernel_end;
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

    // number of 4mb blocks rounded up
    size_t total_4mb = ((lowmem_end + ((1 << 22) - 1))) >> 22;
    size_t pt_count = total_4mb > 0 ? total_4mb - 1 : 0;
    
    if (pt_count > 0) {
        // first pt is already filled
        if (pt_count > (224 - 1))
            pt_count = (224 - 1);

        size_t const pts_bytes = pt_count * 4096;

        uint32_t pt_phys = (uint32_t)e820_alloc(pts_bytes, 1, 1 << 22);

        if (!pt_phys)
            panic();

        uint32_t *pte = (uint32_t *)(pt_phys + KERNEL_VIRT_BASE);

        printk("pt_count=%u (%u bytes)\npt_array=%p\n", pt_count, pts_bytes, pte);

        size_t pt_entries = pt_count * 1024;

        for (size_t i = 0; i < pt_entries; ++i)
            pte[i] = ((1 << 22) + (i * PAGE_SIZE)) | PAGE_RW | PAGE_P;

        for (size_t i = 0; i < pt_count; ++i)
            ((uint32_t *)((char *)pd + KERNEL_VIRT_BASE))[i + 768 + 1] = (pt_phys + i * PAGE_SIZE) | PAGE_RW | PAGE_P;
    }

    // flush tlb
    asm volatile (
        "mov eax, cr3\n\t"
        "mov cr3, eax"
        :
        :
        : "eax", "memory"
    );

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

    if (pfa_init(&mmap))
        panic();

    kmem_cache_init();
    
    inode_init();
    
    dcache_init();
    
    vfs_init();

    int devfs = devfs_init();
    printk("kernel: devfs: init: %i\n", devfs);

    int e9 = e9_init();
    printk("kernel: e9: init: %i\n", e9);

    /*void *heap_start = (void *)((uint32_t)pfa_alloc() + KERNEL_VIRT_BASE);

    kheap_init(heap_start, 4096);

    printk("heap_start=%p, heap_end=%p\n", heap_start, heap_start + 4096 - 1);*/

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
    
    // expose syscall handler
    install_user_interrupt_gate(0x80, &int128_handler);

    printk("kernel: starting init\n");

    // set user stack
    uint32_t stack_phys = (uint32_t)pfa_alloc();

    if (!stack_phys)
        panic();

    uint32_t *stack = (uint32_t *)(KERNEL_VIRT_BASE - 4096);
    
    int map = kmap((void *)(cr3 & ~0xfffu), (void *)stack, (void *)stack_phys, PAGE_US | PAGE_RW | PAGE_P);

    if (map) {
        printk("kmap failed\n");
        for (;;);
    }

    current->ustack_base = (unsigned long)stack;
    current->ustack_size = 4096;

    stack += 1024;

    tss_set_esp0(current->kstack_base + current->kstack_size);

    enter_userland((uint32_t)stack);

    int pid = fork();

    if (pid == 0) {
        int fd = open("/dev/e9", O_RDWR, 0);
        dup2(fd, STDIN);
        dup2(fd, STDOUT);
        dup2(fd, STDERR);

        init();

        close(STDERR);
        close(STDOUT);
        close(STDIN);
    }

    // become idle
    for (;;)
        asm volatile ("pause" :::);
}