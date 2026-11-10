/**
 * @file tss.c
 * @author verner002
 * @date 23/04/2026
*/

#include "kernel/tss.h"

// task state segment
volatile struct tss tss;

void tss_set_esp0(uint32_t stack) {
    tss.esp0 = stack;
}