;
; x86 multiboot loader.
; jump to loader() in loader.cpp to do all the dirty job.
;
; Part of Metta OS. Check http://metta.exquance.com for latest version.
;
; Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
;
; Distributed under the Boost Software License, Version 1.0.
; (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
;
global _start                          ; making entry point visible to linker
extern loader
extern KICKSTART_BASE
extern data_end
extern bss_end

; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0                  ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                  ; provide memory map
KLUDGE      equ  1<<16
FLAGS       equ  MODULEALIGN | MEMINFO | KLUDGE
MAGIC       equ  0x1BADB002            ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)       ; checksum required

bits 32                                ; 32 bit PM

section .bss
align 0x1000
resb 0x1000
initial_stack:                         ; reserve one page for startup stack

section .multiboot_info                ; mboot header should fit in first 8KiB so we make a section for it
align 4
jmp _start
db 'METTALDR',0
align 16
multiboot_header:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    dd multiboot_header
    dd KICKSTART_BASE
    dd data_end                        ; set load_end_addr == 0 to load whole bootloader
    dd bss_end
    dd _start

section .text
_start:
    cli
    cld
    mov esp, initial_stack
    push eax                           ; pass Multiboot flags
    push ebx                           ; pass Multiboot info structure

    mov ebp, 0                         ; make base pointer NULL here so we know
                                       ; where to stop a backtrace.
    call loader                        ; call startup loader code

    cli
    jmp short $                        ; halt machine should startup code return

    mov eax, multiboot_header          ; reference section .multiboot_info so that ld retains it during link

; kate: indent-width 4; replace-tabs on;
; vim: set et sw=4 ts=4 sts=4 cino=(4 :
