;
; Loader
;
; Author: verner002
;

cpu 486
org 0x00008000

%include "const.inc"

bits 16

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

    ;mov si, __data.memory_detect
    ;call __print_str

    ;call __get_mem_size
    ;jc __panic

    ;cmp ebx, 0x00004000 ; we need at least 4 MiB //32 MiB
    ;jb __panic

    ;mov dword [__data.memory_size], ebx
    ;mov eax, ebx
    ;call __print_uint

    ;mov si, __data.kibs
    ;call __print_str

    mov si, __data.construct_smap
    call __print_str

    push 0x0000
    pop es
    mov di, __SMAP_OFFSET
    mov esi, 4096*1024 ;ebx
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
    mov esp, 0x00006000

    call __set_cur_pos32

    mov esi, __data.ok
    call __print_str32

    call __init_paging ; self map first MiB

    mov eax, cr0
    or eax, 0x80000000 ; enable paging
    mov cr0, eax

    mov esi, __data.parse_kernel
    call __print_str32

    ; let's assume, kernel has at most 448 KiB
    ; we create a pt following the self map pt
    ; and start mapping virtual space from
    ; 0x80000000 to kernel's physical address

    mov eax, __SYS_ADDRESS
    call __parse_pe
    jc __panic32

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
    push ebx ; cursor x
    movzx ebx, byte [__cur_pos.cur_y]
    push ebx ; cursor y
    push __PD_OFFSET
    push __SMAP_OFFSET ; smap
    push dword [__data.smap_entries_count] ; smap entries
    ;push dword [__data.memory_size] ; memory size
    
    call eax
    add esp, 0x0000001c ; stack cleanup
    jmp __panic32

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
    jmp short __halt

;
; __panic32
;

__panic32:
    mov si, __data.panic
    call __print_str32
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
    
    ;.memory_size dd 0x00000000
    .smap_entries_count dd 0x00000000