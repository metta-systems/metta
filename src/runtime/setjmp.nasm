;
; Part of Metta OS. Check http://atta-metta.net for latest version.
;
; Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
;
; Distributed under the Boost Software License, Version 1.0.
; (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
;

; Rename functions so they don't clash with whatever builtins there might be.
; extern "C" int __sjljeh_setjmp(jmp_buf buf);
; extern "C" void __sjljeh_longjmp(jmp_buf buf, int retval) NEVER_RETURNS;

global __sjljeh_setjmp
global __sjljeh_longjmp

; FP     -> return address
; FP+4   -> jmp_buf pointer
;
__sjljeh_setjmp:
    mov ecx, [esp]        ; return address to ECX
    mov eax, [esp+4]      ; jmp_buf address to EAX

    mov [eax], ecx        ; first word of jmp_buf = return address

    ; now save GP registers as required by C std
    mov [eax+4], ebx
    mov [eax+8], esi
    mov [eax+12], edi
    mov [eax+16], ebp
    mov [eax+20], esp

    xor eax, eax           ; setjmp returns 0
    ret

; FP     -> return address
; FP+4   -> jmp_buf pointer
; FP+8   -> return value
;
__sjljeh_longjmp:
    mov ecx, [esp+4]       ; jmp_buf address in ECX
    mov eax, [esp+8]       ; return value in EAX

    ; now restore GP registers
    mov ebx, [ecx+4]
    mov esi, [ecx+8]
    mov edi, [ecx+12]
    mov ebp, [ecx+16]
    mov esp, [ecx+20]

    ; restore stack frame
    mov edx, [ecx]          ; original return address in EDX
    mov [esp], edx          ; store original return address
    mov [esp+4], ecx        ; write over jmp_buf address (so that if compiler assumed it to be on the stack frame it won't fail)

    ; return value in EAX, if you longjmp with return value of 0 it's a logic error.
    ret
