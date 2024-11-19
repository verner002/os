;
; EBTFS Header
;
; Author: verner002
;

;
; __ebtfs_header
;

__ebtfs_header:
    .h_magic                dd 0x000eb7f5 ; magic number
    .h_state                db 0
    .h_errors               db 0
    .h_log_blocks_size      db 2 ; 4 KiB per block
    .h_first_data_block     dd 0
    .h_blocks_count         dd 360 ; 360 blocks
    .h_inodes_count         dd 360
    .h_free_blocks_count    dd 360
    .h_free_inodes_count    dd 360
    .h_blocks_per_group     dd 128
    .h_inodes_per_group     dd 128

;
; __void
;

__void:
    .zeros times 1024-($-$$) db 0x00