;
; Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
;
; Distributed under the Boost Software License, Version 1.0.
; (See file LICENSE_1_0.txt or a copy at http:;www.boost.org/LICENSE_1_0.txt)
;
;
; Register.s -- provides functions to read/write registers on the X86
; architecture.
;

global read_instruction_pointer
global read_stack_pointer
global read_base_pointer
global write_stack_pointer
global write_base_pointer
global read_page_directory
global write_page_directory
global flush_page_directory
global enable_paging

read_instruction_pointer:
	pop eax     ; Get the return address
	jmp eax     ; return - can't use RET because return address popped off stack.

read_stack_pointer:
	mov eax, esp
	add eax, 4          ; Stack was pushed with return address, so take into account.
	ret

read_base_pointer:
	mov eax, ebp
	ret

write_stack_pointer:
	pop ebx
	pop eax
	mov esp, eax
	jmp ebx

write_base_pointer:
	mov ebp, [esp+4]
	ret

write_page_directory:
	mov eax, [esp+4]
	mov cr3, eax
	ret

read_page_directory:
	mov eax, cr3
	ret

flush_page_directory:
	mov eax, cr3
	mov cr3, eax
	ret

enable_paging:
	mov eax, cr0
	or  eax, 0x80000000
	mov cr0, eax
	ret

; kate: indent-width 4; replace-tabs on;
; vim: set et sw=4 ts=4 sts=4 cino=(4 :
