;
; Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
;
; Distributed under the Boost Software License, Version 1.0.
; (See file LICENSE_1_0.txt or a copy at http:;www.boost.org/LICENSE_1_0.txt)
;
global activate_gdt
global activate_idt

activate_gdt:
    mov eax, [esp+4]  ; Get the pointer to the GDT, passed as a parameter.
    lgdt [eax]        ; Load the new GDT pointer
    jmp 0x08:.flush   ; 0x08 is the offset to our code segment: Far jump!
.flush:
    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax        ; Load all data segment selectors
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

; TODO: use inline assembly and remove this code
activate_idt:
    mov eax, [esp+4]  ; Get the pointer to the IDT, passed as a parameter.
    lidt [eax]        ; Load the IDT pointer.
    ret

; kate: indent-width 4; replace-tabs on;
; vi:set ts=4:set expandtab=on:
