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
; Input: EAX (pe ptr)
; Output: EAX (entry point), CF (set on error)
;

__parse_pe:
    push eax
    cmp word [eax+__mz_header.magic], __MZ_MAGIC
    jnz .terminate ; invalid mz magic

    add eax, dword [eax+__mz_header.e_lfanew] ; points to pe_signature

    cmp dword [eax+__pe_signature.magic], __PE_MAGIC
    jnz .terminate ; invalid pe magic

    cmp word [eax+__coff_header.machine_type], __MACHINE_I386
    jnz .terminate ; invalid machine type

    cmp word [eax+__opt_header.magic], __OPT_MAGIC
    jnz .terminate ; invalid opt magic

    movzx ecx, word [eax+__coff_header.sections_count]
    mov ebx, dword [eax+__opt_header.image_base]
    movzx esi, word [eax+__coff_header.opt_header_size]
    add esi, __pe_signature_size+__coff_header_size
    add esi, eax ; eax=e_lfanew+base

    ; eax=ptr to pe_signature
    ; ebx=image base
    ; ecx=sections count
    ; esi=ptr to section table

    .map_section:
    call __skip_sections ; TODO: implement support for names longer than 8 chars
    jz .skip_section

    mov edx, dword [esi+__section.virtual_size]

    test edx, edx
    jz .skip_section

    mov edi, dword [esi+__section.virtual_address] ; relative virtual address
    add edi, ebx ; virtual address
    and edi, 0xfffff000 ; make sure address is page aligned

    push eax
    mov ax, dx
    shr edx, 0x0c ; /4096
    and ax, 0x0fff
    setnz al
    movzx eax, al
    add edx, eax ; pages count
    pop eax

    ; eax=ptr to pe_signature
    ; ebx=image base
    ; ecx=sections count
    ; edx=section virtual size
    ; esi=ptr to section table
    ; edi=section virtual address

    .map_page:
    ;call __map_page -- implement (let's assume kernel has up to 4 MiB)
    dec edx
    jnz .map_page

    ; what if __SYS_SEGMENT<<4 == virtual address (skip copying)? (unlikely)

    push ecx
    push edi
    push esi
    mov ecx, dword [esi+__section.raw_data_size]
    mov edi, dword [esi+__section.virtual_address]
    add edi, ebx ; image base
    mov esi, dword [esi+__section.raw_data_ptr]
    add esi, __SYS_ADDRESS ; FIXME: use eax
    rep movsb ; copy raw data
    pop esi
    pop edi
    pop ecx

    .skip_section:
    add esi, __section_size
    loop .map_section
    pop edx ; pe ptr
    add edx, dword [eax+__coff_header.symbol_table_ptr]
    mov ecx, dword [eax+__coff_header.symbols_count]
    mov eax, dword [eax+__opt_header.entry_point]
    add eax, ebx ; image base
    clc
    ret

    .terminate:
    pop eax
    stc
    ret

;
; __skip_sections
;
; Input: EDI (section ptr)
; Output: ZF (set if equal->skip)
;

__skip_sections:
    push edi
    test dword [esi], 0xffff ; long name
    jz .return

    test byte [esi], '/' ; long name (some tools use this format)
    jz .return

    mov edi, .section_comment
    call __match_section
    jz .return

    mov edi, .section_edata
    call __match_section
    jz .return

    mov edi, .section_idata
    call __match_section
    jz .return

    mov edi, .section_rsrc
    call __match_section
    jz .return

    mov edi, .section_tls
    call __match_section
    jz .return

    .return:
    pop edi
    ret

    .section_comment db `.comment`
    .section_edata db `.edata\0\0`
    .section_idata db `.idata\0\0`
    .section_rsrc db `.rsrc\0\0\0`
    .section_tls db `.tls\0\0\0\0`

;
; __match_section
;
; Input: EDI (section ptr), ESI (null-term char arr ptr)
; Output: ZF (set if equal)
;

__match_section:
    ;push ecx
    ;push esi
    ;push edi
    pushad
    mov ecx, 0x00000008 ; section name length
    repz cmpsb
    popad
    ;pop edi
    ;pop esi
    ;pop ecx
    ret

%endif