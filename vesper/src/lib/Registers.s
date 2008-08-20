;
; Register.s -- provides functions to read/write registers on the X86 architecture.
;

global readInstructionPointer
global readStackPointer
global readBasePointer

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
