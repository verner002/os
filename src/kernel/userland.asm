global __get_eip
global __flush_tss
global __exec_kernelmode
global __exec_usermode
extern test_func

__get_eip:
    pop eax
    jmp eax

__flush_tss:
    mov ax, (5 * 8) | 0
    ltr ax
    ret

__exec_kernelmode:
    cli
    mov ebp, esp
    mov ebp, dword [ebp+4]
    mov ax, (2 * 8) | 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; ss is handled by iret

    mov eax, esp
    push (2 * 8) | 0 ; data selector
    push eax ; current stack
    pushf
    pop eax
    or eax, 0x00000200 ; enable interrupts
    push eax ; eflags with if=1
    push (1 * 8) | 0 ; code selector
    push ebp
    iret

__exec_usermode:
    cli
    mov ebp, esp
    mov ebp, dword [ebp+4]

    mov ax, (4 * 8) | 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; ss is handled by iret

    mov eax, esp
    push (4 * 8) | 3 ; data selector
    push eax ; current stack
    pushf
    pop eax
    or eax, 0x00000200 ; enable interrupts
    push eax ; eflags with if=1
    push (3 * 8) | 3 ; code selector
    push ebp
    iret