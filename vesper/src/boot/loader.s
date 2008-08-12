; kate: replace-tabs off; indent-width 3;
global _loader                        ; making entry point visible to linker
extern kernel_entry                   ; kernel_entry is defined elsewhere
extern start_ctors, end_ctors, start_dtors, end_dtors ; c++ init/fini function lists
extern __code, __edata, __end

; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0                 ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                 ; provide memory map
MBOOTVALID  equ  1<<16                ; kernel layout fields are valid
FLAGS       equ  MODULEALIGN | MEMINFO | MBOOTVALID
MAGIC       equ  0x1BADB002           ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)      ; checksum required

; reserve initial kernel stack space
STACKSIZE equ 0x4000                  ; that's 16k.

section .text
align 4
MultiBootHeader:
   dd MAGIC
   dd FLAGS
   dd CHECKSUM
   dd MultiBootHeader
   dd __code
   dd __edata
   dd __end
   dd _loader

_loader:
   cli
   mov esp, stack+STACKSIZE           ; set up the stack
   push eax                           ; pass Multiboot magic number
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

   call  kernel_entry                 ; call kernel proper

static_dtors_loop:
   mov ebx, start_dtors
   jmp short .test
.body:
   call [ebx]
   add ebx,4
.test:
   cmp ebx, end_dtors
   jb .body

   jmp $                              ; halt machine should kernel return

section .bss
align 32
stack:
   resb STACKSIZE                     ; reserve 16k stack on a quadword boundary
