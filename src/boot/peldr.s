;
; @file peldr.s
; @author verner002
; @date 20/11/2025
;

%include "pe.inc"

;
; parse_pe
;

parse_pe:
    push eax
    cmp word [eax+__mz_header.magic], MZ_MAGIC
    jnz .terminate ; invalid mz magic

    add eax, dword [eax+__mz_header.e_lfanew] ; points to pe_signature

    cmp dword [eax+__pe_signature.magic], PE_MAGIC
    jnz .terminate ; invalid pe magic

    cmp word [eax+__coff_header.machine_type], MACHINE_I386
    jnz .terminate ; invalid machine type

    cmp word [eax+__opt_header.magic], OPT_MAGIC
    jnz .terminate ; invalid opt magic

    movzx ecx, word [eax+__coff_header.sections_count]
    mov ebx, dword [eax+__opt_header.image_base]
    movzx esi, word [eax+__coff_header.opt_header_size]
    add esi, __pe_signature_size+__coff_header_size
    add esi, eax ; eax=e_lfanew+base

    ; start of physical location for
    ; parsed pe sections
    mov ebp, 0x00100000

    ; eax=ptr to pe_signature
    ; ebx=image base
    ; ecx=sections count
    ; esi=ptr to section table
    ; ebp=kernel physical address

.map_section:
    call skip_section
    jz .skip_section

    mov edx, dword [esi+__section.virtual_size]

    test edx, edx
    jz .skip_section

    mov edi, dword [esi+__section.virtual_address] ; relative virtual address
    add edi, ebx ; virtual address
    and edi, 0xfffff000 ; make sure address is page aligned

    push edx
    add edx, 0x00000fff
    shr edx, 0x0c

    ; eax=ptr to pe_signature
    ; ebx=image base
    ; ecx=sections count
    ; edx=section virtual size (in pages)
    ; esi=ptr to section table
    ; edi=section virtual address
    ; ebp=kernel physical address

    push edi

.map_page:
    call map_page ; maps up to 4 mib
    add edi, 0x00001000 ; next page
    add ebp, 0x00001000 ; next page
    dec edx
    jnz .map_page
    
    pop edi
    pop edx

    ; copy data
    push ecx
    push edi ; rfc: do i need to push edi???
    push esi
    mov ecx, dword [esi+__section.raw_data_size]
    mov edi, dword [esi+__section.virtual_address]
    add edi, ebx ; image base
    mov esi, dword [esi+__section.raw_data_ptr]
    test esi, esi
    jz .zero ; FIXME: .bss initialization (make it more readable)
    add esi, 0x00011000 ; FIXME: use eax
    rep movsb ; copy raw data
    jmp .popall

.zero:
    push eax
    xor al, al
    mov ecx, edx
    rep stosb
    pop eax

.popall:
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
; skip_section
;

skip_section:
    push edi
    ; has long name?
    test dword [esi], 0xffff
    jz .return

    ; has long name (some tools use this format)
    test byte [esi], '/'
    jz .return

    mov edi, .section_comment
    call match_section
    jz .return

    mov edi, .section_edata
    call match_section
    jz .return

    mov edi, .section_idata
    call match_section
    jz .return

    mov edi, .section_rsrc
    call match_section
    jz .return

    mov edi, .section_tls
    call match_section
    ;jz .return

.return:
    pop edi
    ret

.section_comment    db `.comment`
.section_edata      db `.edata\0\0`
.section_idata      db `.idata\0\0`
.section_rsrc       db `.rsrc\0\0\0`
.section_tls        db `.tls\0\0\0\0`

;
; match_section
;

match_section:
    push ecx
    push esi
    push edi
    ; section name length
    mov ecx, 0x00000008
    repz cmpsb
    pop edi
    pop esi
    pop ecx
    ret

;
; map_page
;

map_page:
    push edi
    shr edi, 22 ; /(4096*1024), pd entry
    cmp edi, 512 ; only this pd entry!!!
    pop edi
    jnz .terminate

    push ebp
    push edi
    shr edi, 12 ; /4096
    and edi, 0x000003ff ; pt entry
    or bp, 0x0007 ; user, r/w, present, 4k
    mov dword [edi*4+0x93000], ebp
    pop edi
    pop ebp

.terminate:
    ret