;
; Legacy Boot Loader
;
; Author: verner002
;

;
; Includes
;

%include "ext2.inc"

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
mov sp, 0x7c00
cld
sti

mov di, SUPERBLOCK_SIZE
mov cx, ax
mov dx, ax
mov ax, di
div word [__drive.bps]
xchg cx, ax

; ax = 1, sector 2 (lba 1)
inc ax
mov bx, 0x7e00 ; es:bx = 0x0000:0x7e00, TODO: use another address
inc cx
call __read_sectors ; read second stage and superblock
jc __panic

; ax points to the first sector of bgdt
push ax
xor dx, dx
mov ax, word [__superblock.total_blocks]
div word [__superblock.group_block_count] ; dx:ax / r/m16 = ax - quotient, dx - remainder
test dx, dx
jz .ok
inc ax

.ok:
mov cx, ax
pop ax

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
.signature dw 0x55aa

;
; __second_stage_void
;

__second_stage_void:
.zeros times 1024-($-$$) db 0x00