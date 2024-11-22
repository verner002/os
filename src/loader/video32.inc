;
; Video
;
; Author: verner002
;

%ifndef __VIDEO32
%define __VIDEO32

%define __VIDEO_MEMORY 0x000b8000
%define __COLUMNS 80
%define __ROWS 25
%define __ARGUMENT 0x07

;
; __cur_pos
;

__cur_pos:
    .cur_x db 0x00
    .cur_y db 0x00

;
; __put_char32
;

__put_char32:
    push ax
    push ebx
    push eax
    mov ax, __COLUMNS
    mul byte [__cur_pos.cur_y]
    movzx ebx, byte [__cur_pos.cur_x]
    add bx, ax ; max. 0xff00 (0xff * 0xff + 0xff)
    shl ebx, 0x01
    add ebx, __VIDEO_MEMORY
    pop eax

    mov ah, __ARGUMENT
    mov word [ebx], ax
    pop ebx
    pop ax
    ret

;
; __print_char32
;

__print_char32:
    push dx
    mov dx, word [__cur_pos]

    cmp al, 0x20
    jb .spec_char

    cmp al, 0xfe ; 0x7e
    ja .spec_char

    call __put_char32

    cmp dl, 0x4f
    jnb .print_nlcr

    inc dl

    .set_cur_pos:
    call __set_cur_pos32

    .return:
    pop dx
    ret

    .spec_char:
    cmp al, 0x0a
    jz .print_nl

    cmp al, 0x0d
    jnz .return

    xor dl, dl
    jmp .set_cur_pos

    .print_nlcr:
    xor dl, dl

    .print_nl:
    cmp dh, 0x18
    jnb .scroll_down

    inc dh
    jmp .set_cur_pos

    .scroll_down:
    call scroll_down
    jmp .set_cur_pos

;
; __print_str32
;
; Input: ESI (null-term char arr ptr)
; Output: Nothing
;

__print_str32:
    push ax
    push esi
    jmp .jump_in

    .__print_char32:
    call __print_char32

    .jump_in:
    lodsb
    test al, al
    jnz .__print_char32
    pop esi
    pop ax
    ret

;
; __print_word32
;
; Input: AX (val)
; Output: Nothing
;

__print_word32:
    pushad
    mov dx, ax
    mov ecx, 0x00000004

    .next_digit:
    rol dx, 0x04
    mov al, dl
    and al, 0x0f

    cmp al, 0x09
    jna .print_digit

    add al, 'A'-'9'-0x01

    .print_digit:
    add al, '0'
    call __print_char32
    loop .next_digit
    popad
    ret

;
; __print_dword32
;
; Input: EAX (val)
; Output: Nothing
;

__print_dword32:
    push eax
    shr eax, 0x10
    call __print_word32
    pop eax
    call __print_word32
    ret

;
; __set_cur_pos32
;
; Input: DH (row), DL (column)
; Output: Nothing
;

__set_cur_pos32:
    mov word [__cur_pos], dx
    call load_cur_loc_regs
    ret

;
; __get_cur_pos32
;
; Input: Nothing
; Output: DH (row), DL (column)
;

__get_cur_pos32:
    mov dx, word [__cur_pos]
    ret

;
; Print Unsigned Integer
;
; Input: EAX (val)
; Output: Nothing
;

print_uint:
push eax
push ebx
push cx
push edx
mov ebx, 0x0000000a
xor cx, cx

.next_digit:
xor edx, edx
div ebx
push dx ; store reminder
inc cx
test eax, eax
jnz .next_digit

.print_digit:
pop ax ; load reminder
add al, '0'
call __print_char32
dec cx
jnz .print_digit
pop edx
pop cx
pop ebx
pop eax
ret

;
; Scroll Down
;
; Input: Nothing
; Output: Nothing
;

scroll_down:
pushad
mov edi, __VIDEO_MEMORY

mov esi, __COLUMNS
mov ecx, esi
shl esi, 0x01
add esi, edi

;movzx ecx, byte [sda.vid_frm_res_x] ; we don't need higher bits

mov al, __ROWS
mul cl
sub ax, cx
push cx
mov cx, ax
rep movsw
pop cx

mov ah, __ARGUMENT
mov al, 0x20
rep stosw
popad
ret

;
; Load Cursor Location Registers
;
; Input: DH (row), DL (column)
; Output: Nothing
;

load_cur_loc_regs:
pusha
movzx ax, dh
mov bl, __COLUMNS
mul bl
movzx bx, dl
add ax, bx

mov cl, 0x0f
mov dx, 0x03d4
call write_ad_reg

mov al, ah
mov cl, 0x0e
call write_ad_reg
popa
ret

;
; Write Address-Data Register
;
; Input: DX (addr reg), CL (index), AL (reg val)
; Output: Nothing
;

write_ad_reg:
push ax
push cx
push dx
mov ch, al
in al, dx ; store addr reg val for later

push ax
mov al, cl
out dx, al
inc dx ; data reg
mov al, ch
out dx, al
dec dx ; addr reg
pop ax

out dx, al ; restore addr reg val
pop dx
pop cx
pop ax
ret

%endif