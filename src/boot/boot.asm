;
; Bootloader
;
; Author: verner002
;

cpu 486
org 0x7c00

bits 16

;
; Constants
;

%define BOOTSEG 0x0000
%define SYSSEG  0x1000

;
; Entry Point
;

jmp short main
nop

%include "fat12.inc"

;
; Main
;

main:
cli
xor ax, ax
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
mov sp, 0x7c00
sti

mov byte [bpb.drv_num], dl

mov dx, 0x03f2
xor al, al
out dx, al ; kill fdd motor

mov ax, word [bpb.num_of_rd_ents]
xor dx, dx
shl ax, 0x05
div word [bpb.bytes_per_sectr]
stosw
mov word [bpb.root_dir_sz], ax

mov ax, word [bpb.sectrs_res]
mov cx, word [bpb.sectrs_per_fat]
mov bx, 0x7e00 ; fat addr
call read_sectrs
jc .panic

xchg ax, cx
mul byte [bpb.num_of_fats]
add ax, cx

mov bx, 0x9000 ; dir buff addr
mov cx, word [bpb.root_dir_sz]
call read_sectrs
jc .panic

mov ax, word [bpb.num_of_rd_ents]
mov si, rodata.loader_sys
mov di, bx

.check_next_rec:
mov cx, 0x0b ; filename length

push si
push di
repz cmpsb
pop di
pop si
jz .loader_found

add di, 0x20 ; record size
dec ax
jnz .check_next_rec

.panic:
mov si, rodata.err
call print_str

.halt:
cli
hlt
jmp .halt

.loader_found:
mov ax, word [bpb.bytes_per_sectr]
movzx cx, byte [bpb.sectrs_per_clust]
mul cx
mov cx, ax

mov ax, word [di+0x001a]
mov bx, 0xac00 ; loader addr

.read_next_clust:
call read_clust
jc .panic

add bx, cx
jc .inc_es

.continue:
call calc_next_clust
jnc .read_next_clust

mov si, rodata.ok
call print_str

jmp 0xac00

.inc_es:
mov dx, es
add dh, 0x10
jz .panic

mov es, dx
jmp .continue

;
; Void
;

void:
.zeros times 510-($-$$) db 0x00
.signature dw 0xaa55