;
; Paging
;
; Author: verner002
;

%ifndef __PAGING
%define __PAGING

%if __PGT_ADDRESS & 0x00000fff != 0
    %error "__PGT_ADDRESS must be page (4096 bytes) aligned!"
%endif

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
    mov edi, __PGT_ADDRESS
    mov ebx, edi

    mov eax, 0x00000002 ; r/w, not present, 4k
    rep stosd ; fill table (4096 bytes, 4 bytes per entry)

    push edi
    .fill_entries: ; pt for first MiB follows the pd
    movzx eax, cl ; let's use cl value to self map the memory
    shl eax, 0x0c ; *4096 (translate to page aligned address)
    or al, 0x03 ; r/w, present
    stosd ; store pt entry
    inc cl
    jnz .fill_entries ; fill 256 entries

    mov ch, 0x03 ; (cx.low=0)
    mov eax, 0x00000002 ; r/w, not present
    rep stosd ; all other pt entries not present
    pop edi

    or di, 0x0003 ; r/w, present, 4k
    mov dword [ebx], edi ; edi contains pt address
    mov cr3, ebx ; load pd address and invalidate cache (ebx.high=0)
    popad
    ret

%endif