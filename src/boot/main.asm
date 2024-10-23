;
; Boot Loader
;
; Author: verner002
;

;
; Includes
;

%include "bfs.inc"

;
; __entry
;

__entry:
jmp 0x0000:main ; fix boot jump to 0x0000:0x7cxx

;
; __main
;

__main:
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

;
; __read_sector
;

__read_sector:
mov ax, 0x0201 ; read one sector
int 0x13
ret

;
; __read_sectors
;

__read_sectors:
ret

;
; __void
;

__void:
.zeros times 510-($-$$) db 0x00
.signature dw 0x55aa