;
; Test'n'Set
;
; Author: verner002
;

global __tas

;
; Test'n'Set
;

__tas:
    push ebp
    mov ebp, esp
    xor eax, eax ; prepare return value
    mov ebx, dword [ebp+8] ; pointer to lock
    lock bts dword [ebx], 0 ; test'n'set lock
    adc eax, 0 ; return carry flag
    pop ebp
    ret
