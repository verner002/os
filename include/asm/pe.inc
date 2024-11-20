;
; Portable Executable
;
; Author: verner002
;

%ifndef MZ_MAGIC
%define MZ_MAGIC 0x5a4d
%endif

;
; MZ Header
;

struc mz_header
    .magic      resw 0x0001
                resb 0x003a
    .e_lfanew   resd 0x0001
endstruc

%ifndef PE_MAGIC
%define PE_MAGIC 0x00004550
%endif

;
; PE Signature
;

struc pe_signature
    .magic  resd 0x0001
endstruc

%ifndef MACH_I386
%define MACH_I386 0x014c
%endif

;
; COFF Header
;

struc coff_header, pe_signature+pe_signature_size
    .machine            resw 0x0001
    .num_of_sects       resw 0x0001
    .time_date_stamp    resd 0x0001
    .ptr_to_sym_tbl     resd 0x0001
    .num_of_syms        resd 0x0001
    .sz_of_opt_hdr      resw 0x0001
    .charact            resw 0x0001
endstruc

%ifndef OPT_MAGIC
%define OPT_MAGIC 0x010b
%endif

%ifndef SUBSYS_NAT
%define SUBSYS_NAT 0x0001
%endif

%ifndef SUBSYS_WGUI
%define SUBSYS_WGUI 0x0002
%endif

%ifndef SUBSYS_WNGUI
%define SUBSYS_WNGUI 0x0003
%endif

%ifndef SUBSYS_OS2
%define SUBSYS_OS2 0x0005
%endif

%ifndef SUBSYS_POSIX
%define SUBSYS_POSIX 0x0007
%endif

%ifndef SUBSYS_HEX
%define SUBSYS_HEX 0xdead
%endif

%ifndef DLL_P_START
%define DLL_P_START 0x0001
%endif

%ifndef DLL_P_END
%define DLL_P_END 0x0002
%endif

%ifndef DLL_T_START
%define DLL_T_START 0x0004
%endif

%ifndef DLL_T_END
%define DLL_P_END 0x0008
%endif

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
    .entry_pt       resd 0x0001
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

%ifndef CH_TEXT
%define CH_TEXT 0x00000020
%endif

%ifndef CH_DATA
%define CH_DATA 0x00000040
%endif

%ifndef CH_BSS
%define CH_BSS 0x00000080
%endif

%ifndef CH_NO_LOAD
%define CH_NO_LOAD 0x00000200
%endif

%ifndef CH_DONT_LINK
%define CH_DONT_LINK 0x00000800
%endif

%ifndef CH_SHARED
%define CH_SHARED 0x10000000
%endif

%ifndef CH_EXEC
%define CH_EXEC 0x20000000
%endif

%ifndef CH_READ
%define CH_READ 0x40000000
%endif

%ifndef CH_WRITE
%define CH_WRITE 0x80000000
%endif

;
; Section
;

struc __section
    .name           resb 0x0008
    .virt_sz        resd 0x0001
    .virt_addr      resd 0x0001
    .sz_of_raw      resd 0x0001
    .ptr_to_raw     resd 0x0001
    .ptr_to_relocs  resd 0x0001
    .ptr_to_ln_nums resd 0x0001
    .num_of_relocs  resw 0x0001
    .num_of_ln_nums resw 0x0001
    .charact        resd 0x0001
endstruc