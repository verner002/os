;
; BIOS Parameter Block
;
; Author: verner002
;

oem_label           db "HEX     "
bytes_per_sectr     dw 0x0200
sectrs_per_clust    db 0x01
sectrs_res          dw 0x0001
num_of_fats         db 0x02
num_of_rd_ents      dw 0x00e0
sectrs_total        dw 0x0b40
media_type          db 0xf0
sectrs_per_fat      dw 0x0009
sectrs_per_track    dw 0x0012
heads_per_cyld      dw 0x0002
num_of_hidd_sectrs  dd 0x00000000
sectrs_total32      dd 0x00000000
drv_num             db 0x00
reserved            db 0x00
signature           db 0x29
serial_num          dd 0xdeadbeef
volume_label        db "HEX        "
file_system         db "FAT12   "