;
; Video
;
; Author: verner002
;

%ifndef __VIDEO
%define __VIDEO

;
; __print_str
;
; Input: (DS:)SI (null-term char arr ptr)
; Output: Nothing
;

__print_str:
    push ax
    push bx
    push si
    mov ah, 0x0e
    xor bh, bh ; mov bx, 0x0007
    jmp .jump_in

    .print_char:
    int 0x10

    .jump_in:
    lodsb
    test al, al
    jnz .print_char
    pop si
    pop bx
    pop ax
    ret

;
; __print_word
;
; Input: AX (val)
; Output: Nothing
;

__print_word:
    pusha
    mov dx, ax
    mov ah, 0x0e
    xor bh, bh ; mov bx, 0x0007
    mov cx, 0x0004

    .next_digit:
    rol dx, 0x04
    mov al, dl
    and al, 0x0f

    cmp al, 0x09
    jna .print_digit

    add al, 'A'-'9'-0x01

    .print_digit:
    add al, '0'
    int 0x10
    loop .next_digit
    popa
    ret

;
; __print_dword
;
; Input: EAX (val)
; Output: Nothing
;

__print_dword:
    push eax
    shr eax, 0x10
    call __print_word
    pop eax
    call __print_word
    ret

;
; Print Unsigned Integer
;
; Input: EAX (val)
; Output: Nothing
;

print_uint16:
push eax
push ebx
push edx
push cx
mov ebx, 0x0000000a
xor cx, cx ; xor cl, cl

.next_digit:
xor edx, edx
div ebx
push dx
inc cx ; inc cl
test eax, eax
jnz .next_digit

;xor bh, bh ; mov bx, 0x0007

.print_digit:
pop ax
add ax, 0x0e00+'0'
int 0x10
dec cx ; dec cl
jnz .print_digit
pop cx
pop edx
pop ebx
pop eax
ret

;
; Print System Memory Map
;
; Input: (ES:)DI (sys mem map struct ptr), CX (num of ents)
; Output: Nothing
;
; Note: Trashes EAX (higher 16 bits).
;

print_sys_mem_map16:
pusha
xor bh, bh ; mov bx, 0x0007
mov si, .padding

.next_ent:
mov eax, dword [es:di] ; base low
call __print_dword

call __print_str

mov eax, dword [es:di+0x0004] ; length low
call __print_dword

call __print_str

push si
mov si, .free

mov eax, dword [es:di+0x0008] ; type
dec eax
jz .print_lbl

mov si, .reserved

.print_lbl:
call __print_str
pop si

add di, 0x0c ; 0x18
loop .next_ent

popa
ret

.padding db `,\0` ;0x20, 0xb3, 0x20, 0x00
.free db `Free\n\r\0`
.reserved db `Reserved\n\r\0`

%endif