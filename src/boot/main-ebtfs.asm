;
; Legacy Boot Loader
;
; Author: verner002
;

cpu 486
org 0x7c00
bits 16

;
; Includes
;

%include "ebtfs.inc"

;
; __entry
;

__entry:
    jmp 0x0000:__main ; fix cs:ip (0x0000:0x7cxx)

;
; __drive
;

__drive:
    .id     db 0x00
    .bps    dw 0x0200
    .spt    dw 0x0012
    .hpc    dw 0x0002

;
; __main
;

__main:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    ;mov fs, ax
    ;mov gs, ax
    mov ss, ax
    mov sp, 0x7c00 ; use 0x7bfe, would be a bit safer?
    cld
    sti

    mov di, 0x0400 ; EBTFS_HEADER_SIZE, but use different name

    mov cx, ax ; cx = 0
    mov dx, ax ; dx = 0
    mov ax, di
    div word [__drive.bps] ; dx:ax / r/m16 = ax - quotient, dx - remainder
    xchg cx, ax ; cx = EBTFS_HEADER_SIZE / __drive.bps (header size in sectors)

    ; ax = 1, sector 2 (lba 1)
    inc ax ; ax = 1
    mov bx, 0x7e00 ; es:bx = 0x0000:0x7e00
    inc cx ; + 1 sector (second stage)
    call __read_sectors ; read second stage and superblock
    jc __panic

    ; check h_magic?
    mov ax, 0x0001

    mov si, di
    mov cl, byte [__ebtfs_header.h_log_blocks_size] ; check if cl <= 5?
    shl si, cl ; check if si == 4 KiB (let's support only this for now)?
    cmp si, di
    ja .block1
    inc ax

    .block1:
    mul si
    div word [__drive.bps] ; ax = first sector of block group descriptor table

    push ax
    xor dx, dx
    mov ax, word [__ebtfs_header.h_blocks_count]
    div word [__ebtfs_header.h_blocks_per_group]
    test dx, dx
    jz .dont_round
    inc ax
    xor dx, dx

    .dont_round:
    shl ax, 0x05 ; ax *= 32
    div word [__drive.bps]
    mov cx, ax
    pop ax

    ; ax points to the first sector of block group descriptor table
    push ax
    xor dx, dx
    mov ax, word [__ebtfs_header.h_blocks_count]
    div word [__ebtfs_header.h_blocks_per_group] ; dx:ax / r/m16 = ax - quotient, dx - remainder
    test dx, dx
    jz .ok
    inc ax

    .ok:
    ; continue

;
; __panic
;

__panic:
    cli
    hlt
    jmp __panic

;
; __read_sectors
;

__read_sectors:
    push ax
    push cx
    xor dx, dx
    div word [__drive.spt]
    inc dx
    mov cl, dl ; sector = lba % spt + 1
    and cl, 0x3f

    xor dx, dx
    div word [__drive.hpc]
    mov dh, dl ; head = lba / spt % hpc
    mov ch, al ; cylinder = lba / spt / hpc
    shl ah, 0x06
    or cl, ah

    mov dl, byte [__drive.id]
    mov ax, 0x0201 ; read one sector
    int 0x13
    pop cx
    pop ax
    jc .return
    inc ax
    add bx, 0x0200 ; word [__drive.bps] ; don't cross page boundary!!!
    loop __read_sectors

    .return:
    ret

;
; __first_stage_void
;

__first_stage_void:
    .zeros times 510-($-$$) db 0x00
    .signature dw 0xaa55

;
; __second_stage_void
;

__second_stage_void:
    .zeros times 1024-($-$$) db 0x00