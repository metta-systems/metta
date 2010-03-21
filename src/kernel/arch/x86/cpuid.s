;
; Provides support for checking and calling cpuid instruction of Intel CPUs.
;
; Part of Metta OS. Check http://metta.exquance.com for latest version.
;
; Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
;
; Distributed under the Boost Software License, Version 1.0.
; (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
;

global processor_cpuid_available
processor_cpuid_available:
    pushfl
    push  ecx
    pushfl
    pop   eax
    mov   ecx, eax
    ; Try flipping the ID bit in EFLAGS.
    xor   eax, 0x200000
    push  eax
    popfl
    ; See if it worked.
    pushfl
    pop   eax
    xor   eax, ecx
    je      1f
    ; If we get here the cpuid instruction is available.
    mov   eax, 1
    pop   ecx
    popfl
    ret
1:
    ; If we get here there is no cpuid instruction.
    xor   eax, eax
    pop   ecx
    popfl
    ret
