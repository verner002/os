;
; FAT12 Bootloader
;
; Author: verner002
;

cpu 486
org 0x7c00

%define __SYS_SEGMENT 0x1000
%define __FAT_OFFSET 0x7e00
%define __RD_OFFSET 0x8000

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
cld
sti

mov byte [drv_num], dl

xor dx, dx
mov ax, word [num_of_rd_ents]
shl ax, 0x05 ; *32
div word [bytes_per_sectr]
mov word [data.rd_sz], ax

mov ax, word [sectrs_res]
mov cx, word [sectrs_per_fat]
mov bx, __FAT_OFFSET
call read_sects
jc .halt

xchg ax, cx
mul byte [num_of_fats]
add ax, cx

mov cx, word [data.rd_sz]
mov bx, __RD_OFFSET
call read_sects
jc .halt

mov ax, word [num_of_rd_ents]
mov si, data.kernel_sys
mov di, bx

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

.halt:
cli
hlt
jmp .halt

.kernel_found:
mov ax, word [di+0x001a]
push __SYS_SEGMENT
pop es
xor bx, bx

.load_kernel:
push ax
sub ax, 0x02

movzx cx, byte [sectrs_per_clust]
mul cx

add ax, word [sectrs_res]

push ax
mov ax, word [sectrs_per_fat]
movzx dx, byte [num_of_fats]
mul dx
mov dx, ax
pop ax

add ax, dx
add ax, word [data.rd_sz]

call read_sects
pop ax
jc .halt

mov bx, ax
shr bx, 0x01
add bx, ax
test ax, 0x01
mov ax, word [bx+__FAT_OFFSET]
jz .even
shr ax, 0x04

.even:
and ah, 0x0f ; 0x000 - 0xfff

cmp ax, 0x0ff8 ; 0xff8 - 0xfff means eof
jb .load_kernel

push __SYS_SEGMENT
push 0x0000
retf ; execute 0x1000:0x0000

;
; Read Sector
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

read_sect:
push ax
push cx
xor dx, dx ; use dx:ax?
div word [sectrs_per_track] ; dx:ax / word -> ax - quotient, dx - remainder
inc dx
mov cl, dl ; sect = lba % spt + 1
and cl, 0x3f ; clear 2 most significant bits (for cylinder)

xor dx, dx
div word [heads_per_cyld] ; dx:ax / word -> ax - quotient, dx - remainder
mov dh, dl ; head = lba / spt % hpc
mov ch, al ; cyld = lba / spt / hpc
shl ah, 0x06 ; 2 most significant cyld bits
or cl, ah

; es:bx - buffer
mov dl, byte [drv_num]
mov di, 0x0003 ; 3 attempts

.again:
mov ax, 0x0201 ; read one sector
stc ; int 0x13 has a problem with setting cf(?)
int 0x13
dec di ; doesn't affect cf
jz .fail
jc .again

.fail:
pop cx
pop ax
ret

;
; Read Sectors
;

read_sects:
pusha

.read_sect:
call read_sect
inc ax
mov bx, word [bytes_per_sectr] ; respect page boundary!!!
loop .read_sect ; cx = 0 is illegal!
popa
ret

;
; Read Cluster
;

read_clust:
push ax
sub ax, 0x02

movzx cx, byte [sectrs_per_clust]
mul cx

add ax, word [sectrs_res]

push ax
mov ax, word [sectrs_per_fat]
movzx dx, byte [num_of_fats]
mul dx
mov dx, ax
pop ax

add ax, dx
add ax, word [data.rd_sz]

call read_sects
pop ax
ret

data:
.rd_sz dw 0x0000
.loader_sys db "LOADER  SYS"
.kernel_sys db "KERNEL  SYS"

times 510-($-$$) db 0x00
dw 0xaa55