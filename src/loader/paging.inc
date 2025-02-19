;
; Paging
;
; Author: verner002
;

%ifndef __PAGING
%define __PAGING

;
; __init_paging
;
; Input: Nothing
; Output: Nothing
;
; Note:
;  Prepares a page directory and self
;  maps the first MiB.
;

__init_paging:
    pushad
    mov ecx, 0x00000400 ; num of pd entries (1024)
    mov edi, __PD_OFFSET
    mov ebx, edi

    mov eax, 0x00000006 ; user, r/w, not present, 4k
    rep stosd ; fill table (4096 bytes, 4 bytes per entry)

    push edi
    .fill_entries: ; pt for first MiB follows the pd
    movzx eax, cl ; let's use cl value to self map the memory
    shl eax, 0x0c ; *4096 (translate to page aligned address)
    or al, 0x07 ; user, r/w, present
    stosd ; store pt entry
    inc cl
    jnz .fill_entries ; fill 256 entries

    mov ch, 0x07 ; 768 fmib entries, 1024 kern pt entries (cx.low=0)
    mov eax, 0x00000006 ; user, r/w, not present
    rep stosd ; all other pt entries not present
    pop edi

    or di, 0x0007 ; user, r/w, present, 4k
    mov dword [ebx], edi ; edi contains fmib pt address
    add edi, 0x00001000 ; kern pt address
    mov dword [ebx+512*4], edi ; edi contains kern pt address
    or ebx, 0x00000008 ; write-through
    mov cr3, ebx ; load pd address and invalidate cache (ebx.high=0)
    popad
    ret

%endif