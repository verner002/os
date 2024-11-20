;
; Global Descriptor Table
;
; Author: verner002
;

%ifndef __GDT
%define __GDT

;
; __set_gdt_limit
;

__set_gdt_limit:
push ebx
push si
mov word [si], bx ; limit.low
shr ebx, 0x10
and bl, 0x0f
add si, 0x06
and byte [si], 0xf0
or byte [si], bl
pop si
pop ebx
ret

;
; __set_gdt_flags
;

__set_gdt_flags:
push cx
shl cl, 0x04
or byte [si+0x0006], cl
pop cx
ret

%endif