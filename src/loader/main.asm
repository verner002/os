;
; Loader
;
; Author: verner002
;

cpu 486
org 0x0000

bits 16

;
; __entry
;

__entry:
    cli
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ;mov ss, ax
    ;mov sp, 0x7c00
    sti

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
    jz __halt

    .a20_enabled:
    mov si, __data.ok
    call __print_str
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
    .ok db `Ok\n\r\0`

%include "video.inc"
%include "memory.inc"