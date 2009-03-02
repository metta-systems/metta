;
; Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
;
; Distributed under the Boost Software License, Version 1.0.
; (See file LICENSE_1_0.txt or a copy at http:;www.boost.org/LICENSE_1_0.txt)
;
global loader                          ; making entry point visible to linker
global initialEsp

extern kernel_entry                    ; kernel_entry is defined elsewhere
extern start_ctors, end_ctors          ; c++ init function lists

; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0                  ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                  ; provide memory map
FLAGS       equ  MODULEALIGN | MEMINFO
MAGIC       equ  0x1BADB002            ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)       ; checksum required

bits 32                                ; 32 bit PM

section .bss
resb 0x1000
initial_stack:                         ; reserve one page for startup stack
initialEsp: resd 1                     ; reserve one dword

section .setup
align 4                                ; mboot header should fit in first 8KiB of ELF image
multiboot_header:                      ; We only include so many fields in the
    dd MAGIC                           ; mboot header because bootloader will
    dd FLAGS                           ; load us as an ELF image.
    dd CHECKSUM

trickgdt:
    dw gdt_end - gdt - 1               ; limit of the GDT
    dd gdt                             ; linear address of GDT

gdt:
    dd 0, 0                            ; null gate
    ; code selector 0x08: base 0x40000000, limit 0xFFFFFFFF, type 0x9A, granularity 0xCF
    db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0x40
    ; data selector 0x10: base 0x40000000, limit 0xFFFFFFFF, type 0x92, granularity 0xCF
    db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0x40
gdt_end:

section .loader.text
align 4
loader:
    lgdt [trickgdt]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; jump to the higher half kernel
    jmp 0x08:higher_half

higher_half:
    ; from now on the CPU will automatically translate every address
    ; by adding the base 0x40000000
    mov esp, initial_stack
    mov [initialEsp], esp              ; record original ESP for remapping the stack later
    push ebx                           ; pass Multiboot info structure

    mov ebp, 0                         ; make base pointer NULL here so we know
                                       ; where to stop a backtrace.
    call  kernel_entry                 ; call kernel proper

    cli
    jmp short $                        ; halt machine should kernel return

; kate: indent-width 4; replace-tabs on;
; vim: set et sw=4 ts=4 sts=4 cino=(4 :
