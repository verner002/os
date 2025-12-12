;
; Switch Task
;
; Author: verner002
;

;
; Exports
;

global __read_eip
global __flush_tss
global __schedule
global __exec_kernelmode
global __exec_usermode
global __yield

extern __update_tick_counter
extern __sched_lock
extern thread_current
extern __dispatch
extern __gdt_set_kstack
extern __send_eoi

;
; __flush_tss
;

__flush_tss:
    mov ax, (5 * 8) | 0
    ltr ax
    ret

;
; __read_ip
;

__read_eip:
    pop eax
    jmp eax

;
; __yield
;

__yield:
    sub esp, 12
    mov dword [esp], eax
    mov eax, dword [esp+12]
    mov dword [esp+4], eax ; eip
    mov eax, cs
    mov dword [esp+8], eax ; cs
    pushfd
    pop eax
    mov dword [esp+12], eax ; flags
    pop eax

    cli
    pushad
    lock bts dword [__sched_lock], 0
    jc .return

    ; load pointer to current task
    mov edx, dword [thread_current]

    ; is there a task?
    test edx, edx
    jz .unlock

    push ds
    push es
    push fs
    push gs

    ; switch to kernel segments
    mov ax, (2 * 8) | 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; save current stack pointer
    mov dword [edx], esp

    call __dispatch

    mov edx, dword [thread_current]
    mov esp, dword [edx]

    push dword [edx+8]
    call __gdt_set_kstack
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    .unlock:
    mov dword [__sched_lock], 0

    .return:
    popad
    iretd

;
; __schedule
;

__schedule:
    ;cli
    pushad
    call __update_tick_counter

    lock bts dword [__sched_lock], 0
    jc .return

    ; load pointer to current task
    mov edx, dword [thread_current]

    ; is there a task?
    test edx, edx
    jz .unlock

    push ds
    push es
    push fs
    push gs

    ; switch to kernel segments
    mov ax, (2 * 8) | 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; save current stack pointer
    mov dword [edx], esp

    call __dispatch

    mov edx, dword [thread_current]
    mov esp, dword [edx]

    push dword [edx+8]
    call __gdt_set_kstack
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    .unlock:
    mov dword [__sched_lock], 0

    .return:
    push 0
    call __send_eoi
    add esp, 4
    popad
    iretd

;
; __exec_kernelmode
;

__exec_kernelmode:
    cli
    mov ebp, esp
    mov ebx, dword [ebp+4] ; eip
    mov esp, dword [ebp+8] ; esp
    mov edx, dword [ebp+16] ; * mutex
    mov ebp, dword [ebp+12] ; ebp

    mov ax, (2 * 8) | 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax ; ss not handled by iret

    ;mov eax, esp
    ;push (2 * 8) | 0 ; data selector
    ;push eax ; current stack pointer
    pushfd
    pop eax
    or eax, 0x00000200 ; enable interrupts
    push eax ; eflags with if=1
    push (1 * 8) | 0 ; code selector
    push ebx
    mov dword [edx], 0 ; unlock mutex
    iretd ; pe=1, nt=0

;
; __exec_usermode
;

__exec_usermode:
    cli
    ;mov ebp, esp
    pop ebx
    ;mov ebx, dword [ebp+4] ; eip
    ;mov esp, dword [ebp+8] ; esp
    ;mov ebp, dword [ebp+12] ; ebp

    mov ax, (4 * 8) | 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; ss is handled by iretd

    ; DataSelector
    ; ESP
    ; EFLAGS
    ; CodeSelector
    ; EIP

    mov eax, esp
    push (4 * 8) | 3 ; data selector
    push eax ; current stack
    pushf
    pop eax
    or eax, 0x00000200 ; enable interrupts
    push eax ; eflags with if=1
    push (3 * 8) | 3 ; code selector
    push ebx
    iretd