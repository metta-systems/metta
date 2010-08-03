/*
 * Copyright (C) 2003-2008,  Karlsruhe University
 * File path:     arch/x86/x32/x86.h
 * Description:   IA32 specific constants
 */
#pragma once

/**********************************************************************
 *    CPU features (CPUID)
 **********************************************************************/

#define X86_32_FEAT_FPU    (1 << 0)
#define X86_32_FEAT_VME    (1 << 1)
#define X86_32_FEAT_DE     (1 << 2)
#define X86_32_FEAT_PSE    (1 << 3)
#define X86_32_FEAT_TSC    (1 << 4)
#define X86_32_FEAT_MSR    (1 << 5)
#define X86_32_FEAT_PAE    (1 << 6)
#define X86_32_FEAT_MCE    (1 << 7)
#define X86_32_FEAT_CXS    (1 << 8)
#define X86_32_FEAT_APIC   (1 << 9)
#define X86_32_FEAT_SEP    (1 << 11)
#define X86_32_FEAT_MTRR   (1 << 12)
#define X86_32_FEAT_PGE    (1 << 13)
#define X86_32_FEAT_MCA    (1 << 14)
#define X86_32_FEAT_CMOV   (1 << 15)
#define X86_32_FEAT_FGPAT  (1 << 16)
#define X86_32_FEAT_PSE36  (1 << 17)
#define X86_32_FEAT_PSN    (1 << 18)
#define X86_32_FEAT_CLFLH  (1 << 19)
#define X86_32_FEAT_DS     (1 << 21)
#define X86_32_FEAT_ACPI   (1 << 22)
#define X86_32_FEAT_MMX    (1 << 23)
#define X86_32_FEAT_FXSR   (1 << 24)
#define X86_32_FEAT_XMM    (1 << 25)
#define X86_32_FEAT_XMM2   (1 << 26)
#define X86_32_FEAT_SS     (1 << 27)
#define X86_32_FEAT_HT     (1 << 28)
#define X86_32_FEAT_TM     (1 << 29)
#define X86_32_FEAT_IA64   (1 << 30)
#define X86_32_FEAT_PBE    (1 << 31)

/* CPUID.1 ECX */
#define X86_32_FEAT2_VMX   (1 << 5)

/**********************************************************************
 *    FLAGS register
 **********************************************************************/

#define X86_FLAGS_CF      (1 <<  0)       /* carry flag                   */
#define X86_FLAGS_PF      (1 <<  2)       /* parity flag                  */
#define X86_FLAGS_AF      (1 <<  4)       /* auxiliary carry flag         */
#define X86_FLAGS_ZF      (1 <<  6)       /* zero flag                    */
#define X86_FLAGS_SF      (1 <<  7)       /* sign flag                    */
#define X86_FLAGS_TF      (1 <<  8)       /* trap flag                    */
#define X86_FLAGS_IF      (1 <<  9)       /* interrupt enable flag        */
#define X86_FLAGS_DF      (1 << 10)       /* direction flag               */
#define X86_FLAGS_OF      (1 << 11)       /* overflow flag                */
#define X86_FLAGS_IOPL(x) ((x & 3) << 12) /* the IO privilege level field */
#define X86_FLAGS_NT      (1 << 14)       /* nested task flag             */
#define X86_FLAGS_RF      (1 << 16)       /* resume flag                  */
#define X86_FLAGS_VM      (1 << 17)       /* virtual 8086 mode            */
#define X86_FLAGS_AC      (1 << 18)       /* alignement check             */
#define X86_FLAGS_VIF     (1 << 19)       /* virtual interrupt flag       */
#define X86_FLAGS_VIP     (1 << 20)       /* virtual interrupt pending    */
#define X86_FLAGS_ID      (1 << 21)       /* CPUID flag                   */
