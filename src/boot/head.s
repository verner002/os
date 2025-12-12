;
; @file head.s
; @author verner002
; @date 18/11/2025
;

cpu 486
org 0

bits 32

;
; entry
;

entry:
    mov ax, 0x0010
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x0009fffe

    ; page directory location
    mov edi, 0x00091000
    mov ebx, edi
    ; 1024 entries
    mov ecx, 0x00000400

    ; user, r/w, not present, 4k
    mov eax, 0x00000006
    ; fill page directory
    rep stosd

    ; fill page table for the
    ; first mib of memory (self
    ; map), edi=0x00092000
    mov eax, edi
    ; user, r/w, present, 4k
    or al, 0x07
    mov dword [ebx], eax
    ;push edi

.self_map:
    movzx eax, cl
    shl eax, 0x0c
    ; user, r/w, present, cache disabled, 4k
    or al, 0x17
    ; fill page table entry
    stosd
    inc cl
    jnz .self_map

    ; remaining entries of the
    ; page table for the first
    ; mib are empty
    mov ch, 0x03
    ; user, r/w, not present, 4k
    mov eax, 0x00000006
    ; fill remaining entries
    rep stosd
    ;pop edi
    ;add edi, 0x1000

    ; fill kernel page table with
    ; empty entries, edi=0x00093000
    mov eax, edi
    ; user, r/w, present, 4k
    or al, 0x07
    mov dword [ebx+512*4], eax

    ; 1024 entries
    mov ecx, 0x00000400
    ; user, r/w, not present, 4k
    mov eax, 0x00000006
    ; fill the page table
    rep stosd

    ; write-through
    or bl, 0x08

    ; load page directory address
    ; and invalidate cache
    mov cr3, ebx

    ; enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; parse kernel image
    mov eax, 0x00001000
    call parse_pe
    jc halt

    ; copy command line with
    ; initial configuration
    mov esi, data.command_line
    mov edi, 0x00090200
    mov ecx, data.command_line_end - data.command_line + 1
    rep movsb

    ; pass control to kernel
    ; TODO: do not use stack
    ;  we don't need it actually
    ;  kernel just has to locate
    ;  system inforation block
    mov edx, dword [0x9000a] ; font address
    push edx
    mov dl, byte [0x90009] ; character rows - 1
    movzx edx, dl
    push edx
    mov dx, word [0x90007] ; bytes per character
    movzx edx, dx
    push edx
    mov dl, byte [0x90000] ; boot drive
    movzx edx, dl
    push edx
    mov dl, byte [0x90002] ; cursor y
    movzx edx, dl
    push edx ; cursor y
    mov dl, byte [0x90001] ; cursor x
    movzx edx, dl
    push edx ; cursor x
    push 0x94000 ; e820 entries
    push dword [0x90003] ; e820 entries count
    call eax
    jmp halt

%include "peldr.s"

;
; halt
;

halt:
    hlt
    jmp short halt

;
; data
;

data:
.smap_entries   dd 0
.command_line db "root=/dev/fd0"
.command_line_end db 0x00
.zeros          times 4096-($-$$) db 0x00