;
; Part of Metta OS. Check http://metta.exquance.com for latest version.
;
; Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
;
; Distributed under the Boost Software License, Version 1.0.
; (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
;
; x86 multiboot loader.
; jump to loader() in loader.cpp to do all the dirty job.
;
global _start                          ; making entry point visible to linker
extern loader
extern _kickstart_begin
extern _data_end
extern _bss_end
extern multiboot_info
extern multiboot_flags

; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0                  ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                  ; provide memory map
KLUDGE      equ  1<<16                 ; we provide bootloader with layout of code and data in mb header
FLAGS       equ  MODULEALIGN | MEMINFO ;| KLUDGE
MAGIC       equ  0x1BADB002            ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)       ; checksum required

bits 32                                ; 32 bit PM

section .bss
align 0x1000
resb 0x1000
initial_stack:                         ; reserve one page for startup stack

section .text
_start:
    cli
    cld

    mov [multiboot_info], ebx          ; pass Multiboot info structure
    mov [multiboot_flags], eax         ; pass Multiboot flags

    mov esp, initial_stack
    xor ebp, ebp                       ; make base pointer NULL here so we know
                                       ; where to stop a backtrace.
    call loader                        ; call startup loader code
                                       ; loader should not return
    cli
    jmp short $                        ; halt machine should startup code return

section .multiboot_info                ; mboot header should fit in first 8KiB so we make a section for it
align 4
multiboot_header:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    dd multiboot_header
    dd _kickstart_begin
    dd _data_end
    dd _bss_end
    dd _start

; kate: indent-width 4; replace-tabs on;
; vim: set et sw=4 ts=4 sts=4 cino=(4 :
