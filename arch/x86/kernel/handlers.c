/**
 * @file handlers.c
 * @author verner002
 * @date 11/05/2026
*/

#include "arch/vga.h"
#include "arch/panic.h"

void division_by_zero_handler(struct interrupt_frame *frame) {
    vga_write("init: fatal: division by zero\n");
    panic();
}

void nonmaskable_interrupt_handler(struct interrupt_frame *frame) {
    vga_write("init: fatal: non-maskable interrupt\n");
    panic();
}

void overflow_handler(struct interrupt_frame *frame) {
    vga_write("init: fatal: overflow\n");
    panic();
}

void bounds_check_handler(struct interrupt_frame *frame) {
    vga_write("init: fatal: bounds check\n");
    panic();
}

void invalid_opcode_handler(struct interrupt_frame *frame) {
    vga_write("init: fatal: invalid opcode\n");
    panic();
}

void device_not_available_handler(struct interrupt_frame *frame) {
    vga_write("init: fatal: device not available\n");
    panic();
}

void double_fault_handler(struct interrupt_frame *frame) {
    vga_write("init: fatal: double fault\n");
    panic();
}

void general_protection_fault_handler(struct interrupt_frame *frame) {
    vga_write("init: fatal: general protection fault\n");
    panic();
}

void page_fault_handler(struct interrupt_frame *frame) {
    vga_write("init: fatal: page fault\n");
    panic();
}

void spurious_interrupt_handler(struct interrupt_frame *frame) {
    vga_write("init: spurious interrupt\n");
}