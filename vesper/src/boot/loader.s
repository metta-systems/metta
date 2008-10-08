; kate: replace-tabs off; indent-width 4; tab-width: 4;
global _loader                         ; making entry point visible to linker
global initialEsp

extern kernel_entry                    ; kernel_entry is defined elsewhere
extern start_ctors, end_ctors          ; c++ init function lists
extern __code, __edata, __end

; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0                  ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                  ; provide memory map
FLAGS       equ  MODULEALIGN | MEMINFO
MAGIC       equ  0x1BADB002            ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)       ; checksum required

bits 32                                ; 32 bit PM

section .data
initialEsp: dd 0

section .text
align 4
MultiBootHeader:                       ; We only include so many fields in the
	dd MAGIC                           ; mboot header because bootloader will
	dd FLAGS                           ; load us as an ELF image.
	dd CHECKSUM

_loader:
	cli
	mov [initialEsp], esp              ; record original ESP for remapping the stack later
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
