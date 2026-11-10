/**
 * @file 8259a.c
 * @author verner002
 * @date 06/05/2026
*/

#include "compiler.h"

#include "kernel/irqs.h"
#include "kernel/cpu.h"
#include "kernel/io.h"
#include "kernel/8259a.h"

#define MASTER_PIC_VECTOR   0x20
#define SLAVE_PIC_VECTOR    0x70

#define MASTER_PIC_CMD      0x0020
#define MASTER_PIC_DATA     0x0021

#define SLAVE_PIC_CMD       0x00a0
#define SLAVE_PIC_DATA      0x00a1

#define EOI                 0x20

// irqs 0h-7h -> int 20h-27h
// irqs 8h-fh -> int 70h-77h
void irqs_init(void) {
    // initialize master pic, icw4
    outb(MASTER_PIC_CMD, 0x11);

    // initialize slave pic, icw4
    outb(SLAVE_PIC_CMD, 0x11);

    // set master's vector offset icw
    outb(MASTER_PIC_DATA, MASTER_PIC_VECTOR);

    // set slave's vector offset icw
    outb(SLAVE_PIC_DATA, SLAVE_PIC_VECTOR);

    // set master's cascade icw
    outb(MASTER_PIC_DATA, 0x04);

    // set slave's cascade icw
    outb(SLAVE_PIC_DATA, 0x02);

    // set master's 80x86 icw
    outb(MASTER_PIC_DATA, 0x01);

    // set slave's 80x86 icw
    outb(SLAVE_PIC_DATA, 0x01);

    // mask all irqs
    outb(SLAVE_PIC_DATA, 0xff);
    outb(MASTER_PIC_DATA, 0xff);
}

static void send_master_eoi(void) {
    outb(MASTER_PIC_CMD, EOI);
}

static void send_slave_eoi(void) {
    outb(SLAVE_PIC_CMD, EOI);
}

void pic_send_eoi(uint8_t irq) {
    if (unlikely(irq >= IRQ_LINES))
        return;

    if (irq >= 8)
        send_slave_eoi();

    send_master_eoi();
}

void pic_enable_irqs(void) {
    outb(MASTER_PIC_DATA, 0x00);
    outb(SLAVE_PIC_DATA, 0x00);
}

void pic_disable_irqs(void) {
    outb(MASTER_PIC_DATA, 0xff);
    outb(SLAVE_PIC_DATA, 0xff);
}

void mask_irq(unsigned int irq) {
    if (unlikely(irq >= IRQ_LINES))
        return;

    uint16_t mask;

    if (irq < 8) {
        mask = inb(MASTER_PIC_DATA);
        mask |= 1 << irq;
        outb(MASTER_PIC_DATA, mask);
    } else {
        irq -= 8;
        mask = inb(SLAVE_PIC_DATA);
        mask |= 1 << irq;
        outb(SLAVE_PIC_DATA, mask);
    }
}

void unmask_irq(unsigned int irq) {
    if (unlikely(irq > 0x0f))
        return;

    uint16_t mask;

    if (irq < 8) {
        mask = inb(MASTER_PIC_DATA);
        mask &= ~(1 << irq);
        outb(MASTER_PIC_DATA, mask);
    } else {
        irq -= 8;
        mask = inb(SLAVE_PIC_DATA);
        mask &= ~(1 << irq);
        outb(SLAVE_PIC_DATA, mask);
    }
}

void disable_irq(unsigned int irq) {
    unsigned int flags;
    
    save_flags(flags);
    cli();
    mask_irq(irq);
    restore_flags(flags);
}

void enable_irq(unsigned int irq) {
    unsigned long flags;
    
    save_flags(flags);
    cli();
    unmask_irq(irq);
    restore_flags(flags);
}

void acknowledge_irq(unsigned int irq) {
    unsigned long flags;
    
    save_flags(flags);
    cli();
    pic_send_eoi(irq);
    restore_flags(flags);
}

/*uint8_t read_master_isr(void) {
    // TODO: we don't need to send ocw3 every time
    //  just keep track of the last command

    // ocw3 read isr
    outb(MASTER_PIC_CMD, 0x0b);
    io_wait();
    return inb(MASTER_PIC_CMD);
}

uint8_t read_slave_isr(void) {
    // TODO: we don't need to send ocw3 every time
    //  just keep track of the last command

    // ocw3 read isr
    outb(SLAVE_PIC_CMD, 0x0b);
    io_wait();
    return inb(SLAVE_PIC_CMD);
}*/