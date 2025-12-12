;
; @file boot.s
; @author verner002
; @date 17/11/2025
;
; @brief File system dependent component that is
;  responsible for loading the setup stage and
;  the kernel image.
;

cpu 486
org 0

%define ORIG_SEG        0x07c0
%define INIT_SEG        0x9000
%define BOOT_SIZE       512
%define FAT_OFFSET      0x8000
%define ROOT_DIR_OFFSET 0xa000

bits 16

;
; The kernel image is limited to 512 KiB since boot and
; setup start at segment 0x9000 but that should be enough
; for micro-kernel.
;

jmp short entry
nop

bpb:
.oem_label              db "OS      "
.bytes_per_sector       dw 0x0200
.sectors_per_cluster    db 0x01
.reserved_sectors       dw 0x0005
.fats_count             db 0x02
.entries_count          dw 0x00e0
.sectors_total          dw 0x0b40
.media_type             db 0xf0
.sectors_per_fat        dw 0x0009
.sectors_per_track      dw 0x0012
.heads_per_cylinder     dw 0x0002
.hidden_sectors_count   dd 0x00000000
.sectors_total_long     dd 0x00000000
.drive_number           db 0x00
.reserved               db 0x00
.signature              db 0x29
.serial_number          dd 0xdeadbeef
.volume_label           db "OS         "
.file_system            db "FAT12   "

;
; entry
;

entry:
    cli
    mov ax, ORIG_SEG
    mov ds, ax
    mov ax, INIT_SEG
    mov es, ax
    cld
    sti

    xor si, si
    xor di, di
    mov cx, BOOT_SIZE / 2
    rep movsw
    jmp far INIT_SEG:main

;
; main
;

main:
    mov ds, ax
    mov ss, ax
    mov sp, 0xfffe

    mov byte [data.drive_number], dl
    mov di, 0x0003

.setup_err:
    mov ax, 0x0204
    mov bx, 0x0200
    mov cx, 0x0002
    xor dx, dx
    stc
    int 0x13 ; read setup
    jnc short .setup_ok

    xor ah, ah
    int 0x13 ; reset fdc
    dec di
    jnz short .setup_err
    jmp short halt

.setup_ok:
    xor dx, dx
    mov ax, word [bpb.entries_count]
    shl ax, 0x05
    div word [bpb.bytes_per_sector]
    mov word [data.root_directory_sectors], ax
    mov word [data.data_area_start], ax

    mov ax, word [bpb.reserved_sectors]
    add word [data.data_area_start], ax
    mov cx, word [bpb.sectors_per_fat]
    mov bx, FAT_OFFSET
    call read_sectors
    jc short halt

    xchg ax, cx
    mul byte [bpb.fats_count]
    mov word [data.fats_sectors], ax
    add word [data.data_area_start], ax
    
    add ax, cx
    mov cx, word [data.root_directory_sectors]
    mov bx, ROOT_DIR_OFFSET
    call read_sectors
    jc short halt

    mov si, data.loading
    call print

    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov si, data.image
    call load_file
    jc short halt

    mov dl, byte [data.drive_number]
    jmp far INIT_SEG:0x0200


;
; halt
;

halt:
    cli
    hlt
    jmp short halt

;
; read_sectors
;

read_sectors:
    push ax
    push cx

.read_sector:
    push ax
    push cx
    xor dx, dx
    div word [bpb.sectors_per_track]
    inc dx
    mov cl, dl
    and cl, 0x3f

    xor dx, dx
    div word  [bpb.heads_per_cylinder]
    mov dh, dl
    mov ch, al
    shl ah, 0x06
    or cl, ah

    mov dl, byte [data.drive_number]
    mov di, 0x0003

.again:
    mov ax, 0x0201
    stc
    int 0x13
    jnc short .ok

    xor ah, ah
    int 0x13
    
    dec di
    jnz short .again
    pop cx
    pop ax
    stc
    jmp short .return
    
.ok:
    pop cx
    pop ax
    inc ax ; RFC: check ax carry overflow?
    mov di, es
    xor dx, dx
    add bx, word [bpb.bytes_per_sector]
    adc dh, 0x0f
    and dh, 0xf0
    add di, dx
    mov es, di
    loop short .read_sector
    ;clc

.return:
    pop cx
    pop ax
    ret

;
; load_file
;

load_file:
    push es
    push ds
    pop es
    mov ax, word [bpb.entries_count]
    mov di, ROOT_DIR_OFFSET

.check_entry:
    mov cx, 0x000b

    push si
    push di
    repz cmpsb
    pop di
    pop si
    jz short .kernel_entry

    add di, 0x20
    dec ax
    jnz short .check_entry
    pop es
    stc
    ret

.kernel_entry:
    mov ax, word [di+0x001a]
    pop es

    mov si, word [data.data_area_start]

.read_cluster:
    push ax
    sub ax, 0x02
    
    movzx cx, byte [bpb.sectors_per_cluster]
    mul cx

    add ax, si
    call read_sectors
    pop ax
    jc short .return

    push bx
    mov bx, ax
    shr bx, 0x01
    add bx, ax
    test ax, 0x01
    mov ax, word [bx+FAT_OFFSET]
    pop bx
    jz short .even
    shr ax, 0x04

.even:
    and ah, 0x0f

    call put_dot

    cmp ax, 0x0ff8
    jb short .read_cluster
    clc

.return:
    ret

;
; put_dot
;

put_dot:
    push ax
    push bx
    mov ah, 0x0e
    mov al, '.'
    xor bh, bh
    int 0x10
    pop bx
    pop ax
    ret

;
; print
;

print:
    push ax
    push bx
    push si
    mov ah, 0x0e
    xor bh, bh
    jmp short .load_char

.put_char:
    int 0x10

.load_char:
    lodsb
    test al, al
    jnz short .put_char

    pop si
    pop bx
    pop ax
    ret

;
; data
;

data:
.drive_number           db 0x00
.root_directory_sectors dw 0x0000
.data_area_start        dw 0x0000
.fats_sectors           dw 0x0000
.loading                db "Loading", 0x00
.image                  db "IMAGE      "
.zeros                  times 510-($-$$) db 0x00
.signature              dw 0xaa55