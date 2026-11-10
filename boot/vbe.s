;
; @file vbe.s
; @author verner002
; @date 20/11/2025
;

;
; vbe_info
;

vbe_info:
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
; vbe_mode_info
;

vbe_mode_info:
    push ax
    mov ax, 0x4f01
    int 0x10

    cmp ax, 0x004f
    pop ax
    ret

;
; set_vbe_mode
;

set_vbe_mode:
    push ax
    mov ax, 0x4f02
    int 0x10

    cmp ax, 0x004f
    pop ax
    ret