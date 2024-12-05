;
; Constants
;
; Author: verner002
;

%define __FAT_OFFSET 0x7e00
%define __RD_OFFSET 0x8000
%define __LDR_OFFSET 0x8000 ; loader will replace root directory
%define __SYS_SEGMENT 0x1000
%define __SMAP_SEGMENT 0x2000
%define __SYS_ADDRESS __SYS_SEGMENT<<4
%define __PGT_ADDRESS 0x0000e000