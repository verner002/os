;
; Memory
;
; Author: verner002
;

%ifndef __MEMORY
%define __MEMORY

;
; __check_a20
;
; Input: Nothing
; Output: CF (set on a20 disabled)
;

__check_a20:
    push es
    push ax
    push si
    ;xor ax, ax
    ;mov ds, ax

    mov ax, 0xffff ; not ax
    mov es, ax
    mov si, 0x7e0e ; if a20 is disabled ds:si will wrap to 0x7dfe

    mov ax, 0xaa55

    cmp word [es:si], ax
    jnz .return ; if byte at ds:si is not 0xaa55 a20 is enabled

    push ax
    xchg ah, al ; 0x55aa
    mov word [0x7dfe], ax ; we should make sure that a20 is disabled
    cmp word [es:si], ax ; if byte at ds:si is not 0x55aa a20 is enabled
    pop ax

    mov word [0x7dfe], ax ; restore original value

    .return:
    pop si
    pop ax
    pop es
    ret

;
; __enable_a20_bios
;
; Input: Nothing
; Output: Nothing
;

__enable_a20_bios:
    push ax
    mov ax, 0x2403 ; query a20 supp
    int 0x15
    jc .return

    test ah, ah
    jnz .return

    mov ax, 0x2401 ; enabled a20
    int 0x15

    .return:
    pop ax
    ret

;
; __enable_a20_ps2c
;
; Input: Nothing
; Output: Nothing
;

__enable_a20_ps2c:
    cli
    push ax
    mov al, 0xad ; disable first ps/2 port
    call __wait_for_ps2_in_buff
    out 0x64, al

    mov al, 0xd0 ; read contr out port
    call __wait_for_ps2_in_buff
    out 0x64, al

    call __wait_for_ps2_out_buff
    in al, 0x60
    or al, 0x02 ; enable a20

    push ax
    mov al, 0xd1 ; write contr out port
    call __wait_for_ps2_in_buff
    out 0x64, al
    pop ax

    call __wait_for_ps2_in_buff
    out 0x60, al

    mov al, 0xae ; enable first ps/2 port
    call __wait_for_ps2_in_buff
    out 0x64, al
    pop ax
    sti
    ret

;
; __enable_a20_port92h
;
; Input: Nothing
; Output: Nothing
;

__enable_a20_port92h:
    cli
    push ax
    in al, 0x92
    or al, 0x02
    out 0x92, al
    pop ax
    stc
    ret

;
; __wait_for_ps2_in_buff
;
; Input: Nothing
; Output: Nothing
;

__wait_for_ps2_in_buff:
    push ax

    .again:
    in al, 0x64
    test al, 0x02
    jnz .again

    pop ax
    ret

;
; __wait_for_ps2_out_buff
;
; Input: Nothing
; Output: Nothing
;

__wait_for_ps2_out_buff:
    push ax

    .again:
    in al, 0x64
    test al, 0x01
    jz .again

    pop ax
    ret

;
; __get_mem_size
;
; Input: Nothing
; Output: EBX (mem size in kibs), CF (set on err)
;
; Note: Returns memory above 16 MiBs only (memory size below 16 MiBs causes error)!!!
;
; TODO: Implement a binary structuring algorithm. The SMAP will contain free
;  and reserved areas and will be sorted.
;

__get_mem_size:
    push ax
    push cx
    push dx
    mov ax, 0xe801 ; handles memory holes under 16 MiBs
    xor cx, cx
    xor dx, dx
    int 0x15 ; get memory size up to 4 GiBs
    jc .terminate

    cmp ah, 0x86
    jz .terminate

    cmp ah, 0x80
    jz .terminate

    jcxz .in_ax_bx

    mov ax, cx
    mov bx, dx

    .in_ax_bx:
    movzx ebx, bx

    cmp ax, 0x0400 ; we want at least 1 MiB in the range of 0 to 16 MiBs
    jb .terminate

    shl ebx, 0x06 ; to KiBs
    jz .terminate

    add ebx, 0x00004000 ; 16 MiBs (let's just assume the memory is here and not use it)
    pop dx
    pop cx
    pop ax
    clc
    ret

    .terminate:
    pop dx
    pop cx
    pop ax
    stc
    ret

;
; __get_smap
;
; Input: (ES:)DI (smap struct ptr), ESI (memory size)
; Output: AX (entries count)
;
; Note:
;  Address Range Descriptor:
;   0x00000000 | dword | base
;   0x00000004 | dword | size
;   0x00000008 | dword | type
;

__get_smap:
    pusha
    mov eax, 0x0000e820 ; get smap
    xor ebx, ebx ; continuation
    mov ecx, 0x00000018 ; buffer size
    mov edx, 'PAMS' ; signature
    mov dword [es:di+0x0014], 0x00000001 ; for pcs with acpi 3.x
    int 0x15
    jc .terminate ; not supported

    cmp eax, 'PAMS' ; we could use edx but some bioses trashes it
    jnz .terminate

    test ebx, ebx
    jz .terminate ; only one descriptor

    xor bp, bp ; address range descriptors counter

    ;mov esi, dword [sda.mem_sz]
    shl esi, 0x0a
    dec esi ; last available address (max. 0xffffffff)
    jmp .check_descriptor

    .next_descriptor:
    mov eax, 0x0000e820 ; get smap
    mov ecx, 0x00000018 ; buffer size
    mov edx, 'PAMS' ; signature
    mov dword [es:di+0x0014], 0x00000001
    int 0x15
    jc .return ; end of list

    .check_descriptor:
    jecxz .skip_descriptor ; invalid descriptor size

    cmp cl, 0x14 ; cl <= 20
    jna .store_descriptor ; check for acpi 3.x descriptor

    test byte [es:di+0x0014], 0x01
    jz .skip_descriptor ; skip ent

    .store_descriptor:
    mov edx, dword [es:di+0x0004] ; base high

    test edx, edx
    jnz .skip_descriptor ; we don't need address range descriptors above 4 GiBs

    mov eax, dword [es:di] ; base low

    cmp eax, esi ; esi contains last mapped address
    ja .skip_descriptor ; skip address range descriptors above detected mem size

    mov edx, dword [es:di+0x000c] ; size high

    test edx, edx
    jnz .correct_size

    mov ecx, dword [es:di+0x0008] ; size low

    test ecx, ecx
    jz .skip_descriptor ; invalid size

    dec ecx
    add ecx, eax ; base low
    ;jc .correct_size ; above 4 GiBs

    cmp ecx, esi
    ja .correct_size

    .size_correct:
    ;mov ecx, dword [es:di+0x0008]
    ;add ecx, dword [es:di]
    ;dec ecx

    ;cmp ecx, esi
    ;ja .skip_descriptor ; skip address range descriptors above detected mem size

    mov ecx, dword [es:di+0x0008] ; size low
    mov dword [es:di+0x0004], ecx
    mov ecx, dword [es:di+0x0010] ; type
    mov dword [es:di+0x0008], ecx

    inc bp
    add di, 0x0c ; 0x18

    .skip_descriptor:
    test ebx, ebx
    jnz .next_descriptor ; end of list

    .return:
    mov word [.buffer], bp
    popa
    mov ax, word [.buffer]
    clc
    ret

    .correct_size:
    xor ecx, ecx
    mov dword [es:di+0x000c], ecx ; size high
    mov ecx, esi
    sub ecx, dword [es:di] ; base low
    inc ecx
    mov dword [es:di+0x0008], ecx ; size low
    jmp .size_correct

    .terminate:
    popa
    stc
    ret

    .buffer dw 0x0000

%endif