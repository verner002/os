;
; Test'n'Set
;
; Author: verner002
;

global __test_set
global __unlock

;
; Test'n'Set
;

__test_set:
    push ebp
    mov ebp, esp
    xor eax, eax ; prepare return value
    mov ebx, dword [ebp+8] ; pointer to lock
    lock bts dword [ebx], 0 ; test'n'set lock
    adc eax, 0 ; return carry flag (TODO: use setc)
    pop ebp
    ret

__unlock:
    push ebp
    mov ebp, esp
    mov eax, dword [ebp+8]
    mov dword [eax], 0
    pop ebp
    ret