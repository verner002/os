;
; Extended File System 2
;
; Author: verner002
;

%ifndef _EXT2_INCLUDED
%define _EXT2_INCLUDED

%define EBTFS_HEADER_SIZE 0x0400 ; always 1024 bytes
%define EBTFS_HEADER_OFF 0x8000

%define FS_IS_CLEAN 0x0001
%define FS_HAS_ERRORS 0x0002

%define IGNORE_ERROR 0x0001
%define REMOUNT_FS_AS_RO 0x0002
%define KERNEL_PANIC 0x0003

struc __ebtfs_header, EBTFS_HEADER_OFF
    .h_magic                resd 1
    .h_state                resb 1
    .h_errors               resb 1
    .h_log_blocks_size      resb 1
    .h_first_data_block     resd 1
    .h_blocks_count         resd 1
    .h_inodes_count         resd 1
    .h_free_blocks_count    resd 1
    .h_free_inodes_count    resd 1
    .h_blocks_per_group     resd 1
    .h_inodes_per_group     resd 1
endstruc

struc inode
    .is_leaf    resb 1
    .min_deg    resw 1
    .no_keys    resw 1
    .keys       resw 1
    .children   resw 1
    .next       resw 1
endstruc

%endif