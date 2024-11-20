;
; Portable Executable
;
; Author: verner002
;

%define MZ_MAGIC 0x5a4d
%define PE_MAGIC 0x00004550
%define MACHINE_I386 0x014c
%define OPT_MAGIC 0x010b

;
; MZ Header
;

struc mz_header
    .magic      resw 0x0001
                resb 0x003a
    .e_lfanew   resd 0x0001
endstruc

;
; PE Signature
;

struc pe_signature
    .magic  resd 0x0001
endstruc

;
; COFF Header
;

struc coff_header, pe_signature+pe_signature_size
    .machine            resw 0x0001
    .num_of_sections    resw 0x0001
    .time_date_stamp    resd 0x0001
    .ptr_to_sym_table   resd 0x0001
    .num_of_syms        resd 0x0001
    .sz_of_opt_header   resw 0x0001
    .charact            resw 0x0001
endstruc

;
; Optional Header
;

struc opt_header, coff_header+coff_header_size
    .magic          resw 0x0001
    .maj_link_ver   resb 0x0001
    .min_link_ver   resb 0x0001
    .sz_of_code     resd 0x0001
    .sz_of_data     resd 0x0001
    .sz_of_bss      resd 0x0001
    .entry_point    resd 0x0001
    .base_of_code   resd 0x0001
    .base_of_data   resd 0x0001
    .img_base       resd 0x0001
    .sect_align     resd 0x0001
    .file_align     resd 0x0001
    .maj_os_ver     resw 0x0001
    .min_os_ver     resw 0x0001
    .maj_img_ver    resw 0x0001
    .min_img_ver    resw 0x0001
    .maj_subsys_ver resw 0x0001
    .min_subsys_ver resw 0x0001
                    resd 0x0001
    .sz_of_img      resd 0x0001
    .sz_of_hdrs     resd 0x0001
    .checksum       resd 0x0001
    .subsystem      resw 0x0001
    .dll_charact    resw 0x0001
    .stack_res      resd 0x0001
    .stack_commit   resd 0x0001
    .heap_res       resd 0x0001
    .heap_commit    resd 0x0001
    .ldr_flags      resd 0x0001
    .num_of_rva_szs resd 0x0001
endstruc

;
; Section
;

struc __section
    .name               resb 0x0008
    .virtual_size       resd 0x0001
    .virtual_address    resd 0x0001
    .size_of_raw        resd 0x0001
    .ptr_to_raw         resd 0x0001
    .ptr_to_relocs      resd 0x0001
    .ptr_to_line_nums   resd 0x0001
    .num_of_relocs      resw 0x0001
    .num_of_line_nums   resw 0x0001
    .charact            resd 0x0001
endstruc