;
; Extended File System 2
;
; Author: verner002
;

%ifndef _EXT2_INCLUDED
%define _EXT2_INCLUDED

%define SUPERBLOCK_SIZE 0x0400 ; 1024 bytes
%define SUPERBLOCK_OFF 0x9000

%define FS_IS_CLEAN 0x0001
%define FS_HAS_ERRORS 0x0002

%define IGNORE_ERROR 0x0001
%define REMOUNT_FS_AS_RO 0x0002
%define KERNEL_PANIC 0x0003

struc __superblock, SUPERBLOCK_OFF
    .total_inodes               resd 1
    .total_blocks               resd 1
    .su_block_count             resd 1
    .total_free_block           resd 1
    .total_free_inodes          resd 1
    .superblock_block_id        resd 1
    .s_log_block_size           resd 1
    .s_log_fragment_size        resd 1
    .group_block_count          resd 1
    .group_fragment_count       resd 1
    .group_inodes_count         resd 1
    .last_mount_time            resd 1
    .last_written_time          resd 1
    .mount_count_since_check    resw 1
    .mount_count_before_check   resw 1
    .ext2_signature             resw 1
    .fs_state                   resw 1
    .what_to_do                 resw 1
    .minor_portion_of_version   resw 1
    .last_check_time            resd 1
    .interval_between_check     resd 1
    .os_id                      resd 1
    .major_portion_of_version   resd 1
    .allow_use_rblock_user_id   resw 1
    .allow_use_rblock_group_id  resw 1
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