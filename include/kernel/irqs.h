/**
 * @file irqs.h
 * @author verner002
 * @date 19/07/2026
*/

#pragma once

#define IRQ_LINES           16

#define TIMER_IRQ           0
#define KEYBOARD_IRQ        1
#define CASCADE_IRQ         2
#define COM2_IRQ            3
#define COM1_IRQ            4
#define LPT2_IRQ            5
#define FLOPPY_IRQ          6
#define LPT1_IRQ            7
#define CMOS_RTC_IRQ        8
#define MOUSE_IRQ           12
#define FPU_IRQ             13
#define PRIMARY_ATA_IRQ     14
#define SECONDARY_ATA_IRQ   15

extern void irqs_init(void);
extern void disable_irq(unsigned int irq);
extern void enable_irq(unsigned int irq);
extern void acknowledge_irq(unsigned int irq);