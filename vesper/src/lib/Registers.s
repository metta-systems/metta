;
; Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at http:;www.boost.org/LICENSE_1_0.txt)
;
;
; Register.s -- provides functions to read/write registers on the X86 architecture.
;

global readInstructionPointer
global readStackPointer
global readBasePointer
global writeStackPointer
global writeBasePointer
global readPageDirectory
global writePageDirectory
global flushPageDirectory:
global enablePaging
global enableInterrupts
global disableInterrupts

readInstructionPointer:
	pop eax     ; Get the return address
	jmp eax     ; return - can't use RET because return address popped off stack.

readStackPointer:
	mov eax, esp
	add eax, 4          ; Stack was pushed with return address, so take into account.
	ret

readBasePointer:
	mov eax, ebp
	ret

writeStackPointer:
	pop ebx
	pop eax
	mov esp, eax
	jmp ebx

writeBasePointer:
	mov ebp, [esp+4]
	ret

writePageDirectory:
	mov eax, [esp+4]
	mov cr3, eax
	ret

readPageDirectory:
	mov eax, cr3
	ret

flushPageDirectory:
	mov eax, cr3
	mov cr3, eax
	ret

enablePaging:
	mov eax, cr0
	or  eax, 0x80000000
	mov cr0, eax
	ret

disableInterrupts:
	cli
	ret

enableInterrupts:
	sti
	ret
