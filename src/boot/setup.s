;
; @file setup.s
; @author verner002
; @date 17/11/2025
;

cpu 486
org 512

bits 16

;
; entry
;

entry:
    mov ax, cs
    mov ds, ax
    mov es, ax

    call test_a20
    jnz .a20_enabled

    call a20_bios
    call test_a20
    jnz .a20_enabled

    call a20_ps2
    call test_a20
    jnz .a20_enabled

    call a20_fast
    call test_a20
    jz halt

.a20_enabled:
    ; 0x0000 - boot drive
    mov byte [0x0000], dl

    ; 0x0001 - cursor x
    ; 0x0002 - cursor y
    mov ah, 0x03
    xor bh, bh
    int 0x10

    mov word [0x0001], dx

    ; 0x0003 - e820 entries count
    ; 0x4000 - e820 entries
    mov di, 0x4000
    call e820_map
    jc halt

    mov dword [0x0003], esi

    mov di, 0x0400
    call vbe_info
    jnz .skip_vbe

    push ds
    mov si, word [es:di+14] ; video modes offset
    mov ax, word [es:di+16] ; video modes segment
    mov ds, ax

.next_mode:
    lodsw
    mov cx, ax
    
    cmp cx, 0xffff
    jz .pop_ds ; 0xffff = end of list

    mov di, 0x0600
    call vbe_mode_info
    jz .mode_info_ok
    jmp .next_mode

.mode_info_ok:
    ; kernel supports all 24-bit modes now
    cmp cx, 0x011b ; 1280×1024, 24-bit (8:8:8)
    jnz .next_mode

    mov bx, cx
    or bx, 0x4000
    ;call set_vbe_mode
    ;jnz .next_mode

    ; get font info
    push es
    mov ax, 0x1130
    mov bh, 0x02 ; get font 8x14
    int 0x10

    mov word [0x0007], cx ; bytes per character
    mov byte [0x0009], dl ; character rows - 1
    xor eax, eax
    mov ax, es
    shl eax, 0x04
    movzx ebp, bp
    add eax, ebp
    mov dword [0x000a], eax ; font address
    pop es

.pop_ds:
    pop ds

.skip_vbe:
    xor eax, eax
    mov ax, ds
    shl eax, 0x04
    add eax, gdt
    mov dword [gdt_ptr+2], eax

    cli
    mov ax, 0x1000
    mov ds, ax

    xor ax, ax
    mov es, ax

    ; move 8 segments
    mov dl, 0x08

    xor si, si
    xor di, di

; TODO: don't perform kernel
;  move, it's useless
.copy:    
    mov cx, 0x8000
    rep movsw

    mov ax, ds
    add ah, 0x10
    mov ds, ax

    mov ax, es
    add ah, 0x10
    mov es, ax

    dec dl
    jnz .copy

    mov ax, cs
    mov ds, ax

    ;lidt [idt_ptr]
    lgdt [gdt_ptr]

    ;
    ; PCN             A W             NE TEMP
    ; GDW             M P             ET SMPE
    ; 0000 0000 0000 0001 0000 0000 0010 0001
    ; 0001 1111 1111 1011 1111 1111 1111 1111
    ; ---------------------------------------
    ; 000x xxxx xxxx x0x1 xxxx xxxx xx1x xxx1
    ; 

    mov eax, cr0
    or eax, 0x00010021
    and eax, 0x1ffbffff
    mov cr0, eax

    ;
    ; TODO: describe cr4
    ;

    ;mov eax, cr4
    ;and ax, 0xffec
    ;mov cr4, eax
    jmp far 0x0008:0x0000

;
; gdt
;

gdt:
    dw 0x0000       ; limit
    dw 0x0000       ; base
    db 0x00         ; base
    db 00000000b    ; access
    db 00000000b    ; flags and limit
    db 0x00         ; base

    dw 0xffff       ; limit
    dw 0x0000       ; base
    db 0x00         ; base
    db 10011010b    ; access
    db 11001111b    ; flags and limit
    db 0x00         ; base

    dw 0xffff       ; limit
    dw 0x0000       ; base
    db 0x00         ; base
    db 10010010b    ; access
    db 11001111b    ; flags and limit
    db 0x00         ; base

;
; gdt_ptr
;

gdt_ptr:
    dw $-gdt-1      ; length
    dd 0            ; pointer

;idt_ptr:
;    dw 0           ; length
;    dd 0           ; pointer

;
; halt
;

halt:
    cli
    hlt
    jmp short halt

%include "vbe.s"

;
; test_a20
;

test_a20:
    push ds
    push es
    xor ax, ax
    mov es, ax
    not ax
    mov ds, ax

    ; 0x00007dfe is safe to
    ; modify
    mov di, 0x7dfe
    mov si, 0x7e0e

    mov ax, word [si]

    ; if values differ a20 is
    ; for sure enabled
    cmp word [es:di], ax
    jnz .return
    
    ; otherwise we perform an
    ; additional test
    push ax
    xchg ah, al
    mov word [si], ax
    cmp word [es:di], ax
    pop ax

    ; and we restore the
    ; previous value
    mov word [si], ax

.return:
    pop es
    pop ds
    ret

;
; a20_bios
;

a20_bios:
    ; a20 service supported?
    mov ax, 0x2403
    int 0x15
    jc .return

    ; failed
    test ah, ah
    jnz .return

    ; get a20 status
    mov ax, 0x2402
    int 0x15
    jc .return

    ; failed
    test ah, ah
    jnz .return

    ; already activated
    test al, 0x01
    jz .return

    ; enable a20
    mov ax, 0x2401
    int 0x15

.return:
    ret

;
; a20_ps2
;

a20_ps2:
    cli
    ; disable first ps2
    mov al, 0xad
    ;call ps2_wait_in
    out 0x64, al

    ; read controller output port
    mov al, 0xd0
    ;call ps2_wait_in
    out 0x64, al

    ;call ps2_wait_out
    ; enable a20
    in al, 0x60
    or al, 0x02

    push ax
    ; write to controller output port
    mov al, 0xd1
    ;call ps2_in
    out 0x64, al
    pop ax

    ;call ps2_wait_in
    out 0x60, al

    ; enable first ps2
    mov al, 0xae
    ;call ps2_wait_in
    out 0x64, al
    sti
    ret

;
; ps2_wait_in
;

ps2_wait_in:
    push ax

.wait:
    in al, 0x64
    test al, 0x02
    jnz .wait

    pop ax
    ret

;
; ps_wait_out
;

ps2_wait_out:
    push ax

.wait:
    in al, 0x64
    test al, 0x01
    jz .wait

    pop ax
    ret

;
; a20_fast
;

a20_fast:
    cli
    in al, 0x92
    or al, 0x02
    out 0x92, al
    stc
    ret

;
; e820_map
;

e820_map:
    mov eax, 0x0000e820
    xor ebx, ebx
    mov ecx, 0x00000018
    mov edx, 'PAMS'
    mov dword [es:di+20], 0x00000001
    int 0x15
    jc .terminate

    cmp eax, 'PAMS'
    jnz .terminate

    xor esi, esi
    jmp .check_ard

.next_ard:
    mov eax, 0x0000e820
    mov ecx, 0x00000018
    mov edx, 'PAMS'
    mov dword [es:di+20], 0x00000001
    int 0x15
    jc .return

.check_ard:
    jecxz .skip_ard

    cmp cl, 0x14
    jna .store_ard

    test byte [es:di+20], 0x01
    jz .skip_ard

.store_ard:
    mov edx, dword [es:di+4] ; base high

    test edx, edx
    jnz .skip_ard

    mov edx, dword [es:di+12] ; size high

    test edx, edx
    jnz .skip_ard

    mov edx, dword [es:di+8] ; size low
    add edx, dword [es:di] ; base low
    jnc .ard_ok ; not likely for 32-bit entries

    sub dword [es:di+8], edx

.ard_ok:
    mov edx, dword [es:di+8] ; size low
    mov dword [es:di+4], edx
    mov edx, dword [es:di+16] ; type
    mov dword [es:di+8], edx

    inc esi
    add di, 0x0c

.skip_ard:
    test ebx, ebx
    jnz .next_ard

.return:
    clc
    ret

.terminate:
    stc
    ret

;
; vga_init
;

vga_init:
    ; map control registers to 0x3dx
    mov dx, 0x03cc
    in al, dx
    or al, 0x01
    mov dx, 0x03c2
    out dx, al

    ; reset 0x03c0 flip-flop
    mov dx, 0x03da
    in al, dx

    ; read address
    mov dx, 0x03c0
    in al, dx
    push ax

    ; select attribute mode control register
    mov al, 0x10
    out dx, al

    ; disable blinking
    inc dx
    in al, dx
    and al, 0xf7
    out dx, al

    ; restore address
    dec dx
    pop ax
    out dx, al
    ret

;
; vbe_init
;

vbe_init:
    ret