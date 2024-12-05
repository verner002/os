;
; FAT12 Bootloader
;
; Author: verner002
;

cpu 486
org 0x7c00

%include "const.inc"

bits 16

;
; __entry
;

__entry:
    jmp short __main
    nop

%include "floppy1440.inc"
;%include "floppy2880.inc"

;
; __main
;

__main:
    ;jmp 0x0000:.main ; correct cs

    ;.main:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    ;mov fs, ax
    ;mov gs, ax
    mov ss, ax
    mov sp, 0x7c00
    cld
    sti

    int 0x13 ; reset disk system
    mov byte [__bpb.drv_num], dl ; store drive number

    xor dx, dx
    mov ax, word [__bpb.num_of_rd_ents]
    shl ax, 0x05 ; *32
    div word [__bpb.bytes_per_sectr]
    mov word [__data.rd_sz], ax ; store root dir size in sectors

    mov ax, word [__bpb.sectrs_res]
    mov cx, word [__bpb.sectrs_per_fat]
    mov bx, __FAT_OFFSET
    call __read_sects ; load fat
    jc __halt

    xchg ax, cx
    mul byte [__bpb.num_of_fats]
    mov word [__data.fats_sz], ax ; store fats size in sectors
    add ax, cx

    mov cx, word [__data.rd_sz]
    mov bx, __RD_OFFSET
    call __read_sects ; load root dir
    jc __halt

    push __SYS_SEGMENT
    pop es
    xor bx, bx
    mov si, __data.kernel_sys
    call __load_file ; load kernel.sys
    jc __halt

    push 0x0000
    pop es
    mov bx, __LDR_OFFSET
    mov si, __data.loader_sys
    call __load_file ; load loader.sys (loader must be the last file to load)
    jc __halt

    ; pass some arguments?
    jmp near __LDR_OFFSET ; execute loader

;
; __halt
;

__halt:
    cli
    hlt
    jmp __halt

;
; __load_file
;

__load_file:
    push es
    push ds
    pop es

    mov ax, word [__bpb.num_of_rd_ents]
    mov di, __RD_OFFSET

    .check_entry:
    mov cx, 0x000b ; filename length

    push si
    push di
    repz cmpsb
    pop di
    pop si
    jz .kernel_found

    add di, 0x20
    dec ax
    jnz .check_entry
    pop es
    stc
    ret

    .kernel_found:
    mov ax, word [di+0x001a]
    pop es

    .load_kernel:
    push ax
    sub ax, 0x02

    movzx cx, byte [__bpb.sectrs_per_clust]
    mul cx

    add ax, word [__bpb.sectrs_res]
    add ax, word [__data.fats_sz]
    add ax, word [__data.rd_sz]

    call __read_sects
    pop ax
    jc .return

    push bx
    mov bx, ax
    shr bx, 0x01
    add bx, ax
    test ax, 0x01
    mov ax, word [bx+__FAT_OFFSET]
    pop bx
    jz .even
    shr ax, 0x04

    .even:
    and ah, 0x0f ; 0x000 - 0xfff

    cmp ax, 0x0ff8 ; 0xff8 - 0xfff means eof
    jb .load_kernel
    clc

    .return:
    ret
;
; __read_sects
;
; int 0x13/ah=0x02, chs:
;
; AX     |    AH    |    AL
; -------|----------|----------
; mode   | 76543210 | 
; count  |          |  6543210
; -------|----------|----------
; BX     |    BH    |    BL
; buffer | 76543210 | 76543210
; -------|----------|----------
; CX     |    CH    |    CL
; cyld   | 76543210 | 98
; sect   |          |   543210
; -------|----------|----------
; DX     |    DH    |    DL
; head   |   543210 |
; drive  |          | 76543210
;

__read_sects:
    push ax
    push cx

    .read_sect:
    push ax
    push cx
    xor dx, dx ; use dx:ax?
    div word [__bpb.sectrs_per_track] ; dx:ax / word -> ax - quotient, dx - remainder
    inc dx
    mov cl, dl ; sect = lba % spt + 1
    and cl, 0x3f ; clear 2 most significant bits (for cylinder)

    xor dx, dx
    div word [__bpb.heads_per_cyld] ; dx:ax / word -> ax - quotient, dx - remainder
    mov dh, dl ; head = lba / spt % hpc
    mov ch, al ; cyld = lba / spt / hpc
    shl ah, 0x06 ; 2 most significant cyld bits
    or cl, ah

    ; es:bx - buffer
    mov dl, byte [__bpb.drv_num]
    mov bp, 0x0003 ; 3 attempts

    .again:
    mov ax, 0x0201 ; read one sector
    stc ; int 0x13 has a problem with setting cf(?)
    int 0x13
    dec bp ; doesn't affect cf
    jz .fail
    jc .again

    .fail:
    pop cx
    pop ax
    jc .return
    inc ax
    mov bp, es
    xor dx, dx
    add bx, word [__bpb.bytes_per_sectr] ; respect page boundary!!!
    adc dh, 0x00 ; setz dh
    shl dh, 0x04
    add bp, dx
    mov es, bp
    loop .read_sect ; cx = 0 is illegal!
    
    .return:
    pop cx
    pop ax
    ret

;
; __data
;

__data:
    .fats_sz dw 0x0000
    .rd_sz dw 0x0000
    .loader_sys db "LOADER  SYS"
    .kernel_sys db "KERNEL  SYS"

;
; __void
;

__void:
    .zeros times 510-($-$$) db 0x00
    .signature dw 0xaa55