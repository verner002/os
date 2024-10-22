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

%define FATSEG  0x0000
%define FATOFF  0x8000
%define DIROFF  0x9000
%define SYSSEG  0x1000
%define SYSOFF  0x0000

;
; Entry Point
;

jmp short main
nop

bpb:
.oem_label          db 'OS      '
.bytes_per_sectr    dw 0x0200
.sectrs_per_clust   db 0x01
.sectrs_res         dw 0x0001
.num_of_fats        db 0x02
.num_of_rd_ents     dw 0x00e0
.sectrs_total       dw 0x0b40
.media_type         db 0xf0
.sectrs_per_fat     dw 0x0009
.sectrs_per_track   dw 0x0012
.heads_per_cyld     dw 0x0002
.num_of_hidd_sectrs dd 0x00000000
.sectrs_total32     dd 0x00000000
.drv_num            db 0x00
.reserved           db 0x00
.signature          db 0x29
.serial_num         dd 0xdeadbeef
.volume_label       db 'OS         '
.file_system        db 'FAT12   '

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
cld
sti

mov byte [bpb.drv_num], dl

mov dx, 0x03f2 ; fdc dor
xor al, al ; kill fdd(s) motor(s)
out dx, al

mov ax, word [bpb.num_of_rd_ents]
xor dx, dx
shl ax, 0x05
div word [bpb.bytes_per_sectr]
stosw
mov word [data.root_dir_sz], ax

mov ax, word [bpb.sectrs_res]
mov cx, word [bpb.sectrs_per_fat]
mov bx, FATOFF
call read_sects
jc .panic

xchg ax, cx
mul byte [bpb.num_of_fats]
add ax, cx

mov bx, DIROFF
mov cx, word [data.root_dir_sz]
call read_sects
jc .panic

mov ax, word [bpb.num_of_rd_ents]
mov si, rodata.loader_sys
mov di, bx

.check_next_rec:
mov cx, 0x000b ; filename length

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
; print an error msg?

.halt:
cli
hlt
jmp .halt

.loader_found:
mov ax, word [di+0x001a]
push SYSSEG
pop es
mov bx, SYSOFF

.read_next_clust:
call read_clust
jc .panic

.continue:
call calc_next_clust
jb .read_next_clust

jmp SYSSEG:SYSOFF

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
xor dx, dx ; use dx:ax?
div word [bpb.sectrs_per_track] ; dx:ax / word -> ax - quotient, dx - remainder
inc dx
mov cl, dl ; sect = lba % spt + 1
and cl, 0x3f ; clear 2 most significant bits (for cylinder)

xor dx, dx
div word [bpb.heads_per_cyld] ; dx:ax / word -> ax - quotient, dx - remainder
mov dh, dl ; head = lba / spt % hpc
mov ch, al ; cyld = lba / spt / hpc
shl ah, 0x06 ; 2 most significant cyld bits
or cl, ah

; es:bx - buffer
mov dl, byte [bpb.drv_num]
mov di, 0x0003 ; 3 attempts

.again:
mov ax, 0x0201 ; read one sector
stc ; int 0x13 has a problem with setting cf(?)
int 0x13
dec di ; doesn't affect cf
jz .fail
jc .again

.fail:
ret

;
; Read Sectors
;

read_sects:
call read_sect
inc ax
add bh, 0x02 ; respect page boundary!!!
loop read_sects ; cx = 0 is illegal!
ret

;
; Read Cluster
;

read_clust:
push ax
sub ax, 0x02

movzx cx, byte [bpb.sectrs_per_clust]
mul cx

add ax, word [bpb.sectrs_res]

push ax
mov ax, word [bpb.sectrs_per_fat]
movzx dx, byte [bpb.num_of_fats]
mul dx
mov dx, ax
pop ax

add ax, dx
add ax, word [data.root_dir_sz]

call read_sects
pop ax
ret

;
; Calculate Next Cluster
;

calc_next_clust:
mov bx, ax
shr bx, 0x01
add bx, ax
test ax, 0x01
mov ax, word [bx+FATOFF]
jz .even
shr ax, 0x04

.even:
and ah, 0x0f ; 0x000 - 0xfff

cmp ax, 0x0ff8 ; 0xff8 - 0xfff means eof
ret

;
; Data
;

data:
.root_dir_sz    dw 0x01e0

;
; Read-Only Data
;

rodata:
.loader_sys db `LOADER  SYS`

;
; Void
;

void:
.zeros times 510-($-$$) db 0x00
.signature dw 0xaa55