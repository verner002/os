/**
 * Kernel
 * 
 * Author: verner002
*/

//#define __DEBUG

/**
 * Includes
*/

#include "null.h"

#include "drivers/cpu.h"
#include "drivers/acpi.h"
#include "drivers/8259a.h"
#include "drivers/8254a.h"
#include "drivers/8042.h"
#include "drivers/8237a.h"
#include "drivers/82077aa.h"

#include "hal/driver.h"
#include "hal/devices.h"
#include "hal/filesystem.h"

//#include "kernel/syms.h"
#include "kernel/e820.h"
#include "kernel/pager.h"
#include "kernel/ts.h"
#include "kernel/task.h"
#include "kernel/fat12.h"

#include "kstdlib/errno.h"
#include "kstdlib/stdio.h"
#include "kstdlib/stdlib.h"

/**
 * Global Variables
*/

TASK *current_task = NULL;

/**
 * panic
*/

void panic(void) {
    printk("\033[31mKERNEL PANIC\n");

    for (;;) {
        asm("cli");
        asm("hlt");
    }
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
    
    asm volatile ("pop %0" : "=m" (eip));

    printk("fault: invalid opcode at %p!\n", eip);
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
    printk("fault: general protection fault, task %u!\n", current_task->pid);
    for (;;);
}

__attribute__((interrupt)) static void __page_fault(INTERRUPT_FRAME *frame) {
    printk("fault: page fault!\n");
    for (;;);
}

__attribute__((interrupt)) void __pit_irq0_handler(INTERRUPT_FRAME *frame) {
    __disable_interrupts();
    
    __send_eoi(0x00);
    
    __update_tick_counter();
    __switch_task();
    
    __enable_interrupts();
}

__attribute__((interrupt)) static void __ps2_irq1_handler(INTERRUPT_FRAME *frame) {
    __send_eoi(0x01);
    __inb(PS2_DATA_PORT_REGISTER);
    printk("\033[33mkbd:\033[37m reading buffer\n");
}

__attribute__((interrupt)) static void syscall(INTERRUPT_FRAME *frame) {
    printk("\033[33mkernel:\033[37m syscall\n");
}

void __user_deamon(void) {
    //printk("\033[33muser:\033[37m USER deamon running, PID=%u\n", __get_pid());

    for (;;);
}

/**
 * entry
*/

void entry(uint32_t e820_entries_count, E820_ENTRY *e820_entries, void *paging_directory, uint32_t cursor_y, uint32_t cursor_x, SYMBOL *symbol_table, uint32_t symbols_count, char *string_table) {
    __init_tick_counter(); // reset tick counter
    
    __init_vga();
    __setcurpos(cursor_y, cursor_x);

    printf("\033[97mWelcome to Kernel!\033[37m\n");

    __init_gdt(0x0010, 0x00007c00);
    
    __sanitize_e820(e820_entries_count, e820_entries);

    __dump_e820();

    if (__init_idt(&__default_isr)) {
        printk("\033[91mFailed to initialize IDT\033[37m\n");
        panic();
    }

    __set_handler(0x00, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__division_by_zero);
    __set_handler(0x02, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__nmi);
    __set_handler(0x04, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__overflow);
    __set_handler(0x05, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__bounds_check);
    __set_handler(0x06, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__invalid_opcode);
    __set_handler(0x07, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__device_not_available);
    __set_handler(0x08, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__double_fault);
    __set_handler(0x0d, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__general_protection_fault);
    __set_handler(0x0e, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__page_fault);

    if (__init_pager()) {
        printk("\033[91mFailed to initialize PMM\033[37m\n");
        panic();
    }

    if (__init_vmm()) {
        printk("\033[91mFailed to initialize VMM\033[37m\n");
        panic();
    }

    // FIXME: don't initialize heap in stdlib
    __init_heap(e820_rmalloc(8192, TRUE), 8192);

    if (__init_acpi()) {
        printk("\033[91mFailed to initialize ACPI\033[37m\n");
        printk("\033[33mkernel:\033[37m \033[96mCannot use APIC => using PIC\033[37m\n");

        // initialize pic
        __init_pics(0x20, 0x70); // irqs 0-7 -> int 20->27, irqs 8-f -> 70->77
        // TODO: irq7 and irq15 must to check isr flag (spurious irqs)
        __disable_irqs();
    } else {
        // initialize apic
    }

    __enable_interrupts(); // we are ready to receive external interrupts
    
    __init_pit();

    // channel 0 for ticks counter, FIXME: doesn't work in bochs
    __outw(PIT_CHANNEL_0_DATA_REGISTER, 0x001234de / 1000); // channel 0 freq=1kHz
    
    __disable_interrupts();
    //__send_master_eoi();
    __set_handler(0x20, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__pit_irq0_handler);
    __enable_interrupts();
    __enable_irq(0x00); // irq0

    // initialize usb controller first
    // ps/2 could be emulated (usb
    // legacy support)
    __init_ps2();

    __disable_interrupts();
    //__send_master_eoi();
    __set_handler(0x21, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__ps2_irq1_handler);
    __enable_interrupts();
    __enable_irq(0x01); // irq1

    // NOTE: vmm test
    void *p = __alloc(4096);
    printf("ptr=%p\n", p);
    panic();

    if (__init_tasking())
        panic();

    if (__init_fdc()) {
        printk("\033[91mFailed to initialize FDC\033[37m\n");
        // ignore? (we can ignore it for a while (until we try to mount root))
    }

    //__create_task((uint32_t)&__user_deamon, TASK_EXEC_USER);
    //__create_task((uint32_t)&__user_deamon);

    /*__fat12_read_fat();
    __fat12_read_root_dir();


    printk("Reading file...\n");
    int32_t last_opcode = __fat12_load_file("KERNEL  SYS", e820_rmalloc(72*1024, FALSE));
    printk("Done: %u\n", last_opcode);*/
    
    if (__init_fsm()) {
        printk("Failed to initialize FSM\n");
        panic();
    }

    DRIVER *fdc_driver = (DRIVER *)malloc(sizeof(DRIVER));

    if (!fdc_driver) {
        printk("Failed to allocate memory for the FDC driver\n");
        panic();
    }

    fdc_driver->symbols = (SYMBOL *)malloc(sizeof(SYMBOL) * 3);
    fdc_driver->symbols[0] = (SYMBOL) {
        .name = "__init",
        .address = &__init_fdc
    };

    printk("fdc::__init: %p\n", __link_symbol(fdc_driver, "__init"));

    // we should be ready to mount root
    DEVICE *root = (DEVICE *)malloc(sizeof(DEVICE));

    if (!root) {
        printk("Failed to allocate memory for root device\n");
        panic();
    }

    root->name = "fd0";
    root->parent = NULL;
    root->driver = fdc_driver;

    char const *root_mp = "/";

    if (__mount(root, root_mp)) {
        printk("Failed to mount device %s to %s\n", root->name, root_mp);
        panic();
    }

    printk("\033[33mkernel:\033[37m Entering IDLE loop\n");

    // idle loop
    for (;;) asm("hlt");

    /*printk("waiting 5 seconds...\n");
    __delay_ms(5000);
    printk("done!\n");*/

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