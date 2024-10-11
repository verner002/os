;
; FAT12 BIOS Parameter Block
;
; Author: verner002
;

istruc bpb
    at bpb.oem_label,           db 'OS      '
    at bpb.bytes_per_sectr,     dw 0x0200
    at bpb.sectrs_per_clust,    db 0x01
    at bpb.sectrs_res,          dw 0x0001
    at bpb.num_of_fats,         db 0x02
    at bpb.num_of_rd_ents,      dw 0x00e0
    at bpb.sectrs_total,        dw 0x0b40
    at bpb.media_type,          db 0xf0
    at bpb.sectrs_per_fat,      dw 0x0009
    at bpb.sectrs_per_track,    dw 0x0012
    at bpb.heads_per_cyld,      dw 0x0002
    at bpb.num_of_hidd_sectrs,  dd 0x00000000
    at bpb.sectrs_total32,      dd 0x00000000
    at bpb.drv_num,             db 0x00
    at bpb.reserved,            db 0x00
    at bpb.signature,           db 0x29
    at bpb.serial_num,          dd 0xdeadbeef
    at bpb.volume_label,        db 'OS         '
    at bpb.file_system,         db 'FAT12   '
iend