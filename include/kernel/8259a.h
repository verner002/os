/**
 * @file 8259a.h
 * @author verner002
 * @date 06/05/2026
*/

#pragma once

#include <kernel/types.h>

void pic_init(void);
void pic_send_eoi(uint8_t irq);
void pic_enable_irqs(void);
void pic_disable_irqs(void);

void disable_irq(unsigned int irq);
void enable_irq(unsigned int irq);