;
; Part of Metta OS. Check http://metta.exquance.com for latest version.
;
; Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
;
; Distributed under the Boost Software License, Version 1.0.
; (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
;
; x86 multiboot loader.
; jump to loader() in loader.cpp to do all the dirty job.
;
global _start                          ; making entry point visible to linker
extern launcher
extern multiboot_info
extern multiboot_flags

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
    call launcher                      ; call launcher code, should not return

    cli
    jmp short $                        ; halt machine should startup code return
