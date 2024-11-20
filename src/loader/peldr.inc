;
; Portable Executable Loader
;
; Author: verner002
;

%ifndef __PELDR
%define __PELDR

%include "pe.inc"

;
; __parse_pe
;
; Input: EDI (pe ptr)
; Output: EAX (addr of sect tbl) EBX (img base), ECX (num of sects), EDX (entry pt), CF (set if inval)
;

__parse_pe:
    push eax
    push ecx
    cmp word [edi+mz_header.magic], MZ_MAGIC
    jnz .terminate ; invalid magic

    mov eax, dword [edi+mz_header.e_lfanew]
    add eax, edi ; load address

    cmp dword [eax+pe_signature.magic], PE_MAGIC
    jnz .terminate ; invalid magic

    cmp word [eax+coff_header.machine], MACH_I386
    jnz .terminate ; invalid machine type

    movzx ecx, word [eax+coff_header.num_of_sects]

    cmp word [eax+opt_header.magic], OPT_MAGIC
    jnz .terminate ; invalid magic

    push ebx
    mov ebx, dword [eax+opt_header.img_base]

    movzx edi, word [eax+coff_header.sz_of_opt_hdr]
    add edi, pe_signature_size+coff_header_size
    add edi, eax ; eax=e_lfanew

    ; EAX - ptr to pe signature
    ; EBX - image base
    ; ECX - num of sections
    ; EDI - section table

    push eax

    .load_section:
    call __skip_sections
    jc .skip_section

    mov edx, dword [edi+__section.virt_sz] ; virt size

    test edx, edx
    jz .skip_section ; skip section if empty

    push ecx ; number of sections
    mov eax, dword [edi+__section.virt_addr] ; relative virtual address
    add eax, ebx ; add image base

    push ebx
    mov ebx, cr3
    ;call __is_pg_mapped ; TODO: i think i can expect the page to be unmapped and remove this
    pop ebx
    jnc .copy ; throw err if mapped?

    mov ecx, edx

    shr ecx, 0x0c ; ecx /= 4096

    and dx, 0x0fff
    jz .alloc

    inc ecx

    .alloc:
    push ebx
    mov ebx, eax ; virtual address

    push edi
    mov edi, 0x00001000

    .map_page:
    ;call __pgalloc
    jz __panic ; some regs are on the stack, TODO: jz .terminate

    push ecx
    mov ecx, cr3
    ;call __map_pg ; eax - set by __pgmalloc, ebx = img_base + section_virt_addr
    pop ecx

    add ebx, edi ; ebx += 4096

    loop .map_page
    pop edi
    pop ebx

    .copy:
    mov ecx, dword [edi+__section.sz_of_raw]

    mov esi, dword [edi+__section.ptr_to_raw]
    ;add esi, KRNL_ADDR

    push edi
    mov edi, dword [edi+__section.virt_addr]
    add edi, ebx ; img base
    rep movsb
    pop edi
    pop ecx

    .skip_section:
    add edi, __section_size ; +sizeof(SECTION)
    loop .load_section
    pop eax

    mov eax, dword [eax+opt_header.entry_pt]
    add eax, ebx ; image base

    pop ebx
    pop ecx
    ;pop eax
    add esp, 0x04 ; clear stack (eax)
    clc
    ret

    .terminate:
    pop ecx
    pop eax
    ;xor eax, eax
    ;mov ebx, eax
    ;mov ecx, ebx
    ;mov edx, ecx
    stc
    ret

;
; __skip_sections
;
; Input: EDI (sect ptr)
; Output: CF (set on skip)
;

__skip_sections:
    push esi
    mov esi, __data.section_comment
    call __match_section
    jz .skip_section

    mov esi, __data.section_edata
    call __match_section
    jz .skip_section

    mov esi, __data.section_idata
    call __match_section
    jz .skip_section

    mov esi, __data.section_rsrc
    call __match_section
    jz .skip_section

    mov esi, __data.section_tls
    call __match_section
    jz .skip_section

    pop esi
    clc
    ret

    .skip_section:
    pop esi
    stc
    ret

;
; __match_section
;
; Input: EDI (sect ptr), ESI (null-term char arr ptr)
; Output: ZF (set if equal)
;

__match_section:
    push ecx
    push esi
    push edi
    mov ecx, 0x00000008 ; section name length
    rep cmpsb
    pop edi
    pop esi
    pop ecx
    ret

%endif