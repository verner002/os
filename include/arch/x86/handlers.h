/**
 * @file handlers.h
 * @author verner002
 * @date 11/05/2026
*/

#pragma once

void division_by_zero_handler(struct interrupt_frame *frame);
void nonmaskable_interrupt_handler(struct interrupt_frame *frame);
void overflow_handler(struct interrupt_frame *frame);
void bounds_check_handler(struct interrupt_frame *frame);
void invalid_opcode_handler(struct interrupt_frame *frame);
void device_not_available_handler(struct interrupt_frame *frame);
void double_fault_handler(struct interrupt_frame *frame);
void general_protection_fault_handler(struct interrupt_frame *frame);
void page_fault_handler(struct interrupt_frame *frame);
void spurious_interrupt_handler(struct interrupt_frame *frame);