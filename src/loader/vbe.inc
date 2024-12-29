;
; VESA BIOS Extension
;
; Author: verner002
;

%ifndef __VBE
%define __VBE

;
; __get_vbe_infor
;

__get_vbe_info:
push ax
push bx
mov ax, 0x4f00
mov dword [es:di], 'VBE2'
int 0x10

cmp ax, 0x004f
jnz .return

cmp dword [es:di], 'VESA'

.return:
pop bx
pop ax
ret

;
; __get_vbe_mode_info
;

__get_vbe_mode_info:
push ax
mov ax, 0x4f01
int 0x10

cmp ax, 0x004f
pop ax
ret

;
; __set_vbe_mode
;

__set_vbe_mode:
push ax
mov ax, 0x4f02
int 0x10

cmp ax, 0x004f
pop ax
ret

%endif