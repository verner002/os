;
; EBTFS Header
;
; Author: verner002
;

;
; __header
;

__header:
    .h_magic                dd 0x000eb7f5
    .h_state                db 0
    .h_errors               db 0
    .h_log_blocks_size      db 0
    .h_first_data_block     dd 0
    .h_blocks_count         dd 1440
    .h_inodes_count         dd 1440
    .h_free_blocks_count    dd 1400
    .h_free_inodes_count    dd 1400
    .h_blocks_per_group     dd 128
    .h_inodes_per_group     dd 128

times 1024-($-$$) db 0x00