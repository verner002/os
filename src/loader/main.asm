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

    push 0x0040
    pop es

    mov ax, 0x0600
    mov bh, 0x07
    xor cx, cx
    mov dh, byte [es:0x0084] ; TODO: store this values for kernel?
    mov dl, byte [es:0x004a] ; it's a word actually, not a byte
    dec dx
    int 0x10

    mov ah, 0x02
    xor bh, bh
    xor dx, dx
    int 0x10

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
    movzx eax, ax
    mov dword [__data.smap_entries_count], eax
    pop es
    jc __panic

    mov esi, __data.ok
    call __print_str

    ;push es
    ;push __SMAP_SEGMENT
    ;pop es
    ;mov cx, ax
    ;call __print_smap
    ;movzx eax, ax
    ;pop es

    mov si, __data.correct_gdt
    call __print_str

    sub ebx, 0x04 ; minus 4k cuz of gdt.limit
    shr ebx, 0x02 ; /4 (page granularity)
    mov cl, 0x0c ; pg granularity, 32-bit descriptor

    mov si, __gdt.code
    call __set_gdt_limit
    call __set_gdt_flags

    mov si, __gdt.data ; add eax, 0x08?
    call __set_gdt_limit
    call __set_gdt_flags

    mov si, __data.ok
    call __print_str

    mov si, __data.enter_pm
    call __print_str

    mov ah, 0x03
    xor bh, bh
    int 0x10 ; dx=cur_pos

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

    jmp 0x0008:__main

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

    .tss:
    dw 0xffff       ; limit
    dw 0x0000       ; base
    db 0x00         ; base
    db 10001001b    ; access byte
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

%define __SYS_SEGMENT 0x8000 ; TODO: use value from same file here and in bootloader

;
; __main
;

__main:
    mov ax, 0x0010
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esi, 0x00007c00

    call __set_cur_pos32

    mov esi, __data.ok
    call __print_str32

    ;lidt [__idt_ptr] -- do in kernel?
    ;sti

    call __init_paging ; self map first MiB

    mov eax, cr0
    or eax, 0x80000000 ; enable paging
    mov cr0, eax

    mov esi, __data.parse_kernel
    call __print_str32

    ; let's assume, kernel has at most 4 MiB
    ; we create a pt following the self map pt
    ; and start mapping virtual space from
    ; 0x80000000 to kernel's physical address

    mov eax, __SYS_SEGMENT<<4
    call __parse_pe
    jc __panic

    mov esi, __data.ok
    call __print_str32

    push eax
    push edx
    mov eax, 0x00000012 ; sizeof(SYMBOL)
    mul ecx
    mov edi, eax
    pop edx
    pop eax
    add edi, edx

    push edi ; string table
    push ecx ; symbols count
    push edx ; symbol table ptr
    movzx ebx, byte [__cur_pos.cur_x]
    push ebx
    movzx ebx, byte [__cur_pos.cur_y]
    push ebx
    push __SMAP_SEGMENT<<4
    push dword [__data.smap_entries_count]
    
    call eax
    ;jmp 0x0008:__SYS_SEGMENT<<4 ; execute kernel

;
; __idt_ptr
;

;__idt_ptr:
;    dw 0x07ff ; length
;    dd __IDT_ADDRESS ; ptr

%include "video32.inc"
%include "paging.inc"
%include "peldr.inc"

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
    .construct_smap db `Constructing SMAP... \0`
    .correct_gdt db `Correcting GDT... \0`
    .enter_pm db `Entering PM... \0`
    .parse_kernel db `Parsing kernel... \0`
    .kibs db ` KiBs\n\r\0`
    .ok db `Ok\n\r\0`
    .panic db `PANIC\n\r\0`
    
    .smap_entries_count dd 0x00000000