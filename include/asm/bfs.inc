;
; B+Tree File System
;
; Author: verner002
;

%ifndef _BFS_INCLUDED
%define _BFS_INCLUDED

struc inode
    .is_leaf    resb 1
    .min_deg    resw 1
    .no_keys    resw 1
    .keys       resw 1
    .children   resw 1
    .next       resw 1
endstruc

%endif