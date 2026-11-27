global enter_userland

enter_userland:
	mov edx, dword [esp+4] ; return address
	mov eax, dword [esp+8] ; user stack

	mov cx, (4 * 8) | 3 ; data selector
	mov ds, cx
	mov es, cx 
	mov fs, cx 
	mov gs, cx ; ss is handled by iret

	push (4 * 8) | 3 ; data selector
	push eax ; current esp
	pushf ; eflags
	push (3 * 8) | 3 ; code selector
	push edx ; original return address
	iretd