global enter_userland

enter_userland:
	pop edx
	mov ax, (4 * 8) | 3 ; data selector
	mov ds, ax
	mov es, ax 
	mov fs, ax 
	mov gs, ax ; ss is handled by iret

	mov eax, esp
	push (4 * 8) | 3 ; data selector
	push eax ; current esp
	pushf ; eflags
	push (3 * 8) | 3 ; code selector
	push edx ; original return address
	iretd