;
; Kernel startup loader.
;
; Part of Metta OS. Check http://metta.exquance.com for latest version.
;
; Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
;
; Distributed under the Boost Software License, Version 1.0.
; (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
;
global _start                          ; making entry point visible to linker
extern kernel_startup

bits 32                                ; 32 bit PM

[section .bss]
align 0x1000
resb 0x1000
initial_stack:                         ; reserve one page for startup stack

[section .text]
_start:
    cli
    cld

    mov esp, initial_stack
    mov ebp, 0                         ; make base pointer NULL here so we know
                                       ; where to stop a backtrace.
    call kernel_startup                ; call kernel startup code
                                       ; loader should not return
    cli
    jmp short $                        ; halt machine should startup code return

; kate: indent-width 4; replace-tabs on;
; vim: set et sw=4 ts=4 sts=4 cino=(4 :
