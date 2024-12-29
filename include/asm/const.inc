;
; Constants
;
; Author: verner002
;

%define __FAT_OFFSET 0x6a00
%define __RD_OFFSET 0x8000
%define __LDR_OFFSET __RD_OFFSET ; loader will replace root directory
%define __SMAP_OFFSET 0xc000
%define __SYS_SEGMENT 0x1000
%define __SYS_ADDRESS __SYS_SEGMENT<<4
%define __PD_OFFSET 0xd000
%define __FMIB_PT_OFFSET __PD_OFFSET+4096 ; must follow pd
%define __KERN_PT_OFFSET __FMIB_PT_OFFSET+4096 ; must follow fmib pt

%if __PD_OFFSET & 0x0fff != 0
    %error "__PD_OFFSET must be page (4096 bytes) aligned!"
%endif

%if __FMIB_PT_OFFSET & 0x0fff != 0
    %error "__FMIB_PT_OFFSET must be page (4096 bytes) aligned!"
%endif

%if __KERN_PT_OFFSET & 0x0fff != 0
    %error "__PD_OFFSET must be page (4096 bytes) aligned!"
%endif