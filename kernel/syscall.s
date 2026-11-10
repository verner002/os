global int80_handler

extern syscall_handler

int80_handler:
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax
    call syscall_handler
    add esp, 4*7
    iretd