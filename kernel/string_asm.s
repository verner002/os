global strlen
global strcpy
global strncpy
global strcmp
global strchr
global memset
global memcpy
global memchr

strlen:
    push ebp
    mov ebp, esp

    push edi

    ; look for terminating zero
    xor eax, eax

    mov ecx, -1

    ; load string pointer into edi
    mov edi, dword [ebp+8]

    ; scan string until zf=1
    repnz scasb

    ; invert bits to determine
    ; number of bytes and substract
    ; one (terminating zero)
    not ecx
    dec ecx

    ; return result in eax
    mov eax, ecx

    pop edi
    pop ebp
    ret

strcpy:
    push ebp
    ; load stack pointer into ebp
    mov ebp, esp

    push edi
    push esi
    
    mov ecx, -1

    ; load destination string pointer into edi
    mov edi, dword [ebp+8]

    ; load source string pointer into esi
    mov esi, dword [ebp+12]

    push edi

.copy:
    ; load byte from source string into al
    lodsb
    ; store byte from al into destination string
    stosb
    ; did we encouter terminating zero?
    test al, al
    jnz .copy

    pop edi
    ; return pointer to destination string
    mov eax, edi

    pop esi
    pop edi
    pop ebp
    ret

strncpy:
    push ebp
    mov ebp, esp

    push edi

    ; load destination string pointer into edi
    mov edi, dword [ebp+8]

    ; load count into ecx
    mov ecx, dword [ebp+16]
    jecxz .return

    push esi
    push edi
    ; load source string pointer into esi
    mov esi, dword [ebp+12]

.copy:
    dec ecx
    jz .restore

    ; load byte from source string into al
    lodsb
    ; store byte from al into destination string
    stosb
    ; did we encouter terminating zero?
    test al, al
    jnz .copy

    ; fill rest of destination string with zeros
    rep stosb

.restore:
    pop edi
    pop esi

.return:
    ; return pointer to destination string
    mov eax, edi

    ; restore esi, edi, ecx and ebp
    pop edi
    pop ebp
    ret

strcmp:
    push ebp
    mov ebp, esp

    push edi
    push esi

    ; load destination string pointer into edi
    mov edi, dword [ebp+12]

    ; load source string pointer into esi
    mov esi, dword [ebp+8]

.compare:
    lodsb
    scasb
    jnz .return

    test al, al
    jnz .compare

    ; there is no difference
    xor eax, eax

    pop esi
    pop edi
    pop ebp
    ret

.return:
    ; substract with borrow
    ; results in -1 for CF=1
    sbb eax, eax
    ; for CF=0 result would be 0
    ; so we just make it 1
    ; for CF=1 nothing changes
    or al, 1

    pop esi
    pop edi
    pop ebp
    ret

strchr:
    push ebp
    mov ebp, esp

    push esi
    mov esi, dword [ebp+8]
    mov ah, byte [ebp+12]

.check:
    lodsb
    cmp al, ah
    jz .return

    test al, al
    jnz .check

    mov esi, 1

.return:
    mov eax, esi
    dec eax
    pop esi
    pop ebp
    ret

memset:
    push ebp
    mov ebp, esp

    push edi

    mov edi, dword [ebp+8]
    mov eax, dword [ebp+12]
    mov ecx, dword [ebp+16]

    push edi
    rep stosb
    pop edi
    
    mov eax, edi

    pop edi
    pop ebp
    ret

memcpy:
    push ebp
    mov ebp, esp

    push edi
    push esi

    mov edi, dword [ebp+8]
    mov esi, dword [ebp+12]
    mov ecx, dword [ebp+16]
    
    push edi
    rep movsb
    pop edi

    mov eax, edi

    pop esi
    pop edi
    pop ebp
    ret

memchr:
    push ebp
    mov ebp, esp

    push edi
    mov edi, dword [ebp+8]
    mov al, byte [ebp+12]
    mov ecx, dword [ebp+16]
    jecxz .zero

    repnz scasb
    jz .return

.zero:
    mov edi, 1

.return:
    mov eax, edi
    dec eax
    pop edi
    pop ebp
    ret