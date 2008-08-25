; kate: replace-tabs off; indent-width 4; tab-width: 4;
global _loader                         ; making entry point visible to linker

extern kernel_entry                    ; kernel_entry is defined elsewhere
extern start_ctors, end_ctors          ; c++ init function lists
extern __code, __edata, __end

; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0                  ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                  ; provide memory map
MBOOTVALID  equ  1<<16                 ; kernel layout fields are valid
FLAGS       equ  MODULEALIGN | MEMINFO | MBOOTVALID
MAGIC       equ  0x1BADB002            ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)       ; checksum required

bits 32                                ; 32 bit PM

section .text
align 4
MultiBootHeader:
	dd MAGIC
	dd FLAGS
	dd CHECKSUM
	dd MultiBootHeader                 ; Location of this descriptor
	dd __code                          ; Start of kernel code
	dd __edata                         ; End of kernel "data" section.
	dd __end                           ; End of kernel.
	dd _loader                         ; Entry point.

_loader:
	cli
	push ebx                           ; pass Multiboot info structure

static_ctors_loop:
	mov ebx, start_ctors
	jmp short .test
.body:
	call [ebx]
	add ebx,4
.test:
	cmp ebx, end_ctors
	jb .body

	mov ebp, 0                         ; make base pointer NULL here so we know
	                                   ; where to stop a backtrace.
	call  kernel_entry                 ; call kernel proper

	cli
	jmp $                              ; halt machine should kernel return
