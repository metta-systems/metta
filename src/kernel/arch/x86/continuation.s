;
; Part of Metta OS. Check http://metta.exquance.com for latest version.
;
; Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
;
; Distributed under the Boost Software License, Version 1.0.
; (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
;
global asm_activate

; FIXME: doesn't set fs and gs - potential security hole!

; void asm_activate(gpregs_t* gpregs, uint32_t cs, uint32_t ds)
; [ESP+4] = ptr
asm_activate:
    mov edi, [esp + 4]  ; gpregs
    mov ecx, [esp + 8]  ; cs
    mov edx, [esp + 12] ; ds
    push edx            ; ss3
    mov eax, [edi + 36] ; esp3
    push eax
    mov eax, [edi + 32] ; eflags
    push eax
    push ecx            ; user cs
    mov eax, [edi + 28] ; eip
    push eax
    ; Other regs
    mov eax, [edi + 12] ; edx
    push eax            ; current edx contains DS
    mov eax, [edi + 0] ; eax
    mov ebx, [edi + 4] ; ebx
    mov ecx, [edi + 8] ; ecx
    mov esi, [edi + 16] ; esi
    mov ebp, [edi + 24] ; ebp
    mov edi, [edi + 20] ; edi
    mov ds, dx
    mov es, dx
    pop edx
    iret                ; jump to user mode
