/**
 * @file gdt.c
 * @author verner002
 * @date 23/04/2026
*/

#include <string.h>

#include <kernel/gdt.h>
#include <kernel/tss.h>

// global descriptor table
volatile struct gdt gdt[6];
volatile struct gdt_ptr gdt_ptr;

void gdt_init(void) {
    // null descriptor
    gdt[0] = (struct gdt) {
        .limit_low = 0,
        .base_low = 0,
        .accessed = 0,
        .read_write = 0,
        .direction_conforming = 0,
        .executable = 0,
        .descriptor_type = 0,
        .descriptor_privilage_level = 0,
        .present = 0,
        .limit_high = 0,
        .reserved1 = 0,
        .long_mode = 0,
        .size = 0,
        .granularity = 0,
        .base_high = 0
    };

    // kernelmode code descriptor
    gdt[1] = (struct gdt) {
        .limit_low = 0xffff,
        .base_low = 0x000000,
        .accessed = 0,
        .read_write = 1,
        .direction_conforming = 0,
        .executable = 1,
        .descriptor_type = 1,
        .descriptor_privilage_level = 0,
        .present = 1,
        .limit_high = 0xf,
        .reserved1 = 0,
        .long_mode = 0,
        .size = 1,
        .granularity = 1,
        .base_high = 0x00
    };

    // kernelmode data descriptor
    gdt[2] = (struct gdt) {
        .limit_low = 0xffff,
        .base_low = 0x000000,
        .accessed = 0,
        .read_write = 1,
        .direction_conforming = 0,
        .executable = 0,
        .descriptor_type = 1,
        .descriptor_privilage_level = 0,
        .present = 1,
        .limit_high = 0xf,
        .reserved1 = 0,
        .long_mode = 0,
        .size = 1,
        .granularity = 1,
        .base_high = 0x00
    };

    // userland code descriptor
    gdt[3] = (struct gdt) {
        .limit_low = 0xffff,
        .base_low = 0x000000,
        .accessed = 0,
        .read_write = 1,
        .direction_conforming = 0,
        .executable = 1,
        .descriptor_type = 1,
        .descriptor_privilage_level = 3,
        .present = 1,
        .limit_high = 0xf,
        .reserved1 = 0,
        .long_mode = 0,
        .size = 1,
        .granularity = 1,
        .base_high = 0x00
    };

    // userland data descriptor
    gdt[4] = (struct gdt) {
        .limit_low = 0xffff,
        .base_low = 0x000000,
        .accessed = 0,
        .read_write = 1,
        .direction_conforming = 0,
        .executable = 0,
        .descriptor_type = 1,
        .descriptor_privilage_level = 3,
        .present = 1,
        .limit_high = 0xf,
        .reserved1 = 0,
        .long_mode = 0,
        .size = 1,
        .granularity = 1,
        .base_high = 0x00
    };

    uint32_t base = (uint32_t)&tss;
    uint32_t size = sizeof(struct tss);

    // tss, used by userland tasks when
    // performing system calls
    gdt[5] = (struct gdt) {
        .limit_low = size,
        .base_low = base,
        .accessed = 1,
        .read_write = 0,
        .direction_conforming = 0,
        .executable = 1,
        .descriptor_type = 0,
        .descriptor_privilage_level = 0,
        .present = 1,
        .limit_high = (size & 0x000f0000) >> 16,
        .reserved1 = 0,
        .long_mode = 0,
        .size = 0,
        .granularity = 0,
        .base_high = (base & 0xff000000) >> 24
    };

    // clear tss
    memset((void *)&tss, 0, size);

    // kernel stack segment
    // tss.esp0 is set during task switch
    tss.ss0 = KERNEL_DATA_SEGMENT;

    // no io bitmap
    tss.io_map_base = size;

    gdt_ptr.length = sizeof(gdt) - 1;
    gdt_ptr.base = (uint32_t)&gdt;

    asm volatile (
        "lgdt %0"
        :
        : "m" (gdt_ptr)
        :
    ); // flush gdt

    flush_tss();
}