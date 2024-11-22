;
; Portable Executable
;
; Author: verner002
;

%define __MZ_MAGIC 0x5a4d
%define __PE_MAGIC 0x00004550
%define __MACHINE_I386 0x014c
%define __OPT_MAGIC 0x010b

;
; MZ Header
;

struc __mz_header
    .magic      resw 0x0001
                resb 0x003a
    .e_lfanew   resd 0x0001
endstruc

;
; PE Signature
;

struc __pe_signature
    .magic  resd 0x0001
endstruc

;
; COFF Header
;

struc __coff_header, __pe_signature+__pe_signature_size
    .machine_type           resw 0x0001
    .sections_count         resw 0x0001
    .time_date_stamp        resd 0x0001
    .symbol_table_ptr       resd 0x0001
    .symbols_count          resd 0x0001
    .opt_header_size        resw 0x0001
    .characteristic         resw 0x0001
endstruc

;
; Optional Header
;

struc __opt_header, __coff_header+__coff_header_size
    .magic                  resw 0x0001
    .maj_link_version       resb 0x0001
    .min_link_version       resb 0x0001
    .code_size              resd 0x0001
    .data_size              resd 0x0001
    .bss_size               resd 0x0001
    .entry_point            resd 0x0001
    .code_base              resd 0x0001
    .data_base              resd 0x0001
    .image_base             resd 0x0001
    .section_align          resd 0x0001
    .file_align             resd 0x0001
    .maj_os_version         resw 0x0001
    .min_os_version         resw 0x0001
    .maj_image_version      resw 0x0001
    .min_image_version      resw 0x0001
    .maj_subsys_version     resw 0x0001
    .min_subsys_version     resw 0x0001
                            resd 0x0001
    .image_size             resd 0x0001
    .headers_size           resd 0x0001
    .checksum               resd 0x0001
    .subsys_type            resw 0x0001
    .dll_characteristics    resw 0x0001
    .stack_reserve_size     resd 0x0001
    .stack_commit_size      resd 0x0001
    .heap_reserve_size      resd 0x0001
    .heap_commit_size       resd 0x0001
    .loader_flags           resd 0x0001
    .rva_and_sizes_count    resd 0x0001
endstruc

;
; Section
;

struc __section
    .name               resb 0x0008
    .virtual_size       resd 0x0001
    .virtual_address    resd 0x0001
    .raw_data_size      resd 0x0001
    .raw_data_ptr       resd 0x0001
    .relocs_ptr         resd 0x0001
    .line_numbers_ptr   resd 0x0001
    .relocs_count       resw 0x0001
    .line_numbers_count resw 0x0001
    .characteristics    resd 0x0001
endstruc