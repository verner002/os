
push eax

.load_section:
call __skip_sections
jc .skip_section

mov edx, dword [esi+__section.virtual_size]

test edx, edx
jz .skip_section ; skip empty section

push ecx ; number of sections
mov eax, dword [esi+__section.virtual_address] ; relative virtual address
add eax, ebx ; add image base

;push ebx
;mov ebx, cr3
;call __is_pg_mapped ; TODO: i think i can expect the page to be unmapped and remove this
;pop ebx
;jnc .copy ; throw err if mapped?

mov ecx, edx

shr ecx, 0x0c ; ecx /= 4096

and dx, 0x0fff
jz .alloc

inc ecx

.alloc:
push ebx
mov ebx, eax ; virtual address

push edi
mov edi, 0x00001000

.map_page:
;call __pgalloc
jz __panic ; some regs are on the stack, TODO: jz .terminate

push ecx
mov ecx, cr3
;call __map_pg ; eax - set by __pgmalloc, ebx = img_base + section_virt_addr
pop ecx

add ebx, edi ; ebx += 4096

loop .map_page
pop edi
pop ebx

.copy:
mov ecx, dword [edi+__section.sz_of_raw]

mov esi, dword [edi+__section.ptr_to_raw]
;add esi, KRNL_ADDR

push edi
mov edi, dword [edi+__section.virt_addr]
add edi, ebx ; img base
rep movsb
pop edi
pop ecx

.skip_section:
add edi, __section_size ; +sizeof(SECTION)
loop .load_section
pop eax

mov eax, dword [eax+opt_header.entry_pt]
add eax, ebx ; image base

pop ebx
pop ecx
;pop eax
add esp, 0x04 ; clear stack (eax)
clc
ret

.terminate:
pop ecx
pop eax
;xor eax, eax
;mov ebx, eax
;mov ecx, ebx
;mov edx, ecx
stc
ret