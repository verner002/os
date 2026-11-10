/**
 * @file exec.h
 * @author verner002
 * @date 04/11/2025
*/

#pragma once

#include <kernel/types.h>

#define MID_I386            0x86

#define OMAGIC              0x107
#define NMAGIC              0x108
#define ZMAGIC              0x10b

#define N_GETFLAG(ex)       (((ex).a_midmag >> 26) & 0x3f)
#define N_GETMID(ex)        (((ex).a_midmag >> 16) & 0x3ff)
#define N_GETMAGIC(ex)      (((ex).a_midmag >> 0) & 0xffff)

struct exec {
    uint32_t a_midmag;
    uint32_t a_text;        // size of text segment
    uint32_t a_data;        // size of data segment
    uint32_t a_bss;         // size of bss segment
    uint32_t a_syms;        // size of symbol table
    uint32_t a_entry;       // entry point address
    uint32_t a_trsize;      // size of text relocation table
    uint32_t a_drsize;      // size of data relocation table
};