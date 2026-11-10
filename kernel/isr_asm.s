global int32_handler
global trampoline
global int128_handler
global syscall_return

extern acknowledge_irq
extern do_timer

extern syscall_table
extern NSYSCALL

extern init
extern sys_create

ENOSYS      equ 38

int32_handler:
    push ebp
    mov ebp, esp

    pushad
    push 0
    call acknowledge_irq
    add esp, 4

    mov eax, dword [ebp+8] ; cs
    and eax, 3
    push eax
    call do_timer
    add esp, 4

    popad
    pop ebp
    iretd

trampoline:
    iretd

extern notify
extern init

int128_handler:
    pushad

    cmp eax, dword [NSYSCALL]
    jnb bad_syscall

    push esp
    call dword [syscall_table+eax*4]
    add esp, 4

syscall_return:
    mov dword [esp+28], eax
    popad
    iretd

bad_syscall:
    popad
    mov eax, -ENOSYS
    iretd