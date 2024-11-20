;
; Loader
;
; Author: verner002
;

cpu 486
org 0x00008000

bits 16

%define __SMAP_SEGMENT 0x2000
%define __PGT_ADDRESS 0xe000

;
; __entry
;

__entry:
    ;cli
    ;xor ax, ax
    ;mov ds, ax
    ;mov es, ax
    ;mov fs, ax
    ;mov gs, ax
    ;mov ss, ax
    ;mov sp, 0x7c00
    ;sti

    mov si, __data.enable_a20
    call __print_str

    call __check_a20
    jnz .a20_enabled

    call __enable_a20_bios
    call __check_a20
    jnz .a20_enabled

    call __enable_a20_ps2c
    call __check_a20
    jnz .a20_enabled

    call __enable_a20_port92h
    call __check_a20
    jz __panic

    .a20_enabled:
    mov si, __data.ok
    call __print_str

    mov si, __data.memory_detect
    call __print_str

    call __get_mem_size
    jc __panic

    mov eax, ebx
    call __print_uint

    mov si, __data.kibs
    call __print_str

    mov si, __data.construct_smap
    call __print_str

    xor di, di

    push es
    push __SMAP_SEGMENT
    pop es
    mov esi, ebx
    call __get_smap
    pop es
    jc __panic

    push es
    push __SMAP_SEGMENT
    pop es
    mov cx, ax
    call __print_smap
    pop es

    sub ebx, 0x04 ; minus 4k cuz of gdt.limit
    shr ebx, 0x02 ; /4 (page granularity)
    mov cl, 0x0c ; pg granularity, 32-bit descriptor

    mov si, __gdt.code
    call __set_gdt_limit
    call __set_gdt_flags

    mov si, __gdt.data ; add eax, 0x08?
    call __set_gdt_limit
    call __set_gdt_flags

    cli
    lgdt [__gdt_ptr]

    ; CR0:
    ; 0000 0000 0000 0001 0000 0000 0010 0001
    ; 0001 1111 1111 1101 1111 1111 1111 1111
    ; PCN             A W             NE TEMP
    ; GDW             M P             ET SMPE

    mov eax, cr0
    or eax, 0x00010021 ; disable paging, write protect and enable protected mode, fpu exceptions are processed by internal mechanism
    and eax, 0x1ffdffff ; enable cache fills, enable write-through, disable align error
    
    mov cr0, eax

    mov eax, cr4
    and ax, 0xffec ; disable pse, pvi and vme
    mov cr4, eax

    jmp 0x0008:__main ; FIXME: loader must reside in first segment!!!

;
; __gdt
;

__gdt:
    .null:
    dw 0x0000       ; limit
    dw 0x0000       ; base
    db 0x00         ; base
    db 00000000b    ; access byte
    db 00000000b    ; flags and limit
    db 0x00         ; base

    .code:
    dw 0xffff       ; limit
    dw 0x0000       ; base
    db 0x00         ; base
    db 10011010b    ; access byte
    db 11001111b    ; flags and limit
    db 0x00         ; base

    .data:
    dw 0xffff       ; limit
    dw 0x0000       ; base
    db 0x00         ; base
    db 10010010b    ; access byte
    db 11001111b    ; flags and limit
    db 0x00         ; base

;
; __gdt_ptr
;

__gdt_ptr:
    dw $-__gdt-1 ; length
    dd __gdt ; ptr

%include "video.inc"
%include "memory.inc"
%include "gdt.inc"

bits 32

%define __SYS_SEGMENT 0x8000

;
; __main
;

__main:
    ;lidt [__idt_ptr]
    ;sti

    call __init_paging

    mov eax, cr0
    or eax, 0x80000000 ; enable paging
    mov cr0, eax

    ; TODO: parse kernel image

    push __SYS_SEGMENT
    push 0x000
    retf ; execute kernel

;
; __idt_ptr
;

;__idt_ptr:
;    dw 0x07ff ; length
;    dd __IDT_ADDRESS ; ptr

%include "paging.inc"

;
; __panic
;

__panic:
    mov si, __data.panic
    call __print_str
    ;jmp short __halt

;
; __halt
;

__halt:
    cli
    hlt
    jmp __halt

;
; __data
;

__data:
    .enable_a20 db `Enabling A20... \0`
    .memory_detect db `Detecting memory... \0`
    .construct_smap db `Constructing SMAP...\n\r\0`
    .kibs db ` KiBs\n\r\0`
    .ok db `Ok\n\r\0`
    .panic db `PANIC\n\r\0`