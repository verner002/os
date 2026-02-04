;
; @file head.s
; @author verner002
; @date 18/11/2025
;

cpu 486

bits 32

extern main
extern __bss_start
extern __bss_size
global _start

;
; start
;

_start:
    mov ax, 0x0010
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x0008fffe

    ; page directory location
    mov edi, 0x00081000
    mov ebx, edi
    ; 1024 entries
    mov ecx, 0x00000400

    ; user, r/w, not present, 4k
    mov eax, 0x00000006
    ; fill page directory
    rep stosd

    ; fill page table for the first
    ; 4 MiB of memory (direct mapping)
    ; edi=0x00082000
    mov eax, edi
    ; user, r/w, present, cache disabled, 4k
    or al, 0x17
    mov dword [ebx], eax
    ;push edi

.direct_map:
    movzx eax, cx
    shl eax, 0x0c
    ; user, r/w, present, cache disabled, 4k
    or al, 0x17
    ; fill page table entry
    stosd
    inc cx
    cmp cx, 0x0400
    jb .direct_map

    ; write-through
    or bl, 0x08

    ; load page directory address
    ; and invalidate cache
    mov cr3, ebx

    ; enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; copy command line with
    ; initial configuration
    mov esi, data.command_line
    mov edi, 0x00080200
    mov ecx, data.command_line_end - data.command_line + 1
    rep movsb

    ; clear bss section
    xor eax, eax
    mov edi, __bss_start
    mov ecx, __bss_size
    rep stosd

    ; RFC: copy 512-byte system
    ;  information block from
    ;  0x90000 to 0x80000?
    call main
    add esp, 32 ; clear stack
    ;jmp halt


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