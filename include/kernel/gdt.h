/**
 * @file gdt.h
 * @author verner002
 * @date 23/04/2026
*/

#pragma once

#define KERNEL_CODE_SEGMENT (sizeof(struct gdt) * 1)
#define KERNEL_DATA_SEGMENT (sizeof(struct gdt) * 2)
#define USER_CODE_SEGMENT   (sizeof(struct gdt) * 3)
#define USER_DATA_SEGMENT   (sizeof(struct gdt) * 4)
#define TASK_STATE_SEGMENT  (sizeof(struct gdt) * 5)

#include "kernel/types.h"

struct __attribute__((packed)) gdt_ptr {
    uint16_t length;
    uint32_t base;
};

struct __attribute__((packed)) gdt {
    uint32_t limit_low                  : 16;
    uint32_t base_low                   : 24;
    uint32_t accessed                   : 1;
    uint32_t read_write                 : 1;
    uint32_t direction_conforming       : 1;
    uint32_t executable                 : 1;
    uint32_t descriptor_type            : 1;
    uint32_t descriptor_privilage_level : 2;
    uint32_t present                    : 1;
    uint32_t limit_high                 : 4;
    uint32_t reserved1                  : 1;
    uint32_t long_mode                  : 1;
    uint32_t size                       : 1;
    uint32_t granularity                : 1;
    uint32_t base_high                  : 8;
};

extern volatile struct gdt gdt[6];
extern volatile struct gdt_ptr gdt_ptr;

void gdt_init(void);
void tss_set_esp0(uint32_t stack);