global ctxsw

ctxsw:
    push ebp
    mov ebp, esp
    
    pushfd
    pushad
    
    ; store segment registers

    ; pointer to location of old stack
    mov eax, dword [ebp+8]
    ; store old stack value
    mov dword [eax], esp

    ; pointer to location of new stack
    mov eax, dword [ebp+12]
    ; restore new stack value
    mov esp, dword [eax]

    ; restore segment registers

    ; load page directory (and flags)
    mov eax, dword [ebp+16]
    mov cr3, eax

    popad
    ; we restore ebp before possibly
    ; enabling interrupts
    mov ebp, dword [esp+4]
    popfd
    ; skip ebp
    add esp, 4
    ret