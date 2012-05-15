//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/**
 * Nucleus is the privileged part of the whole system.
 * It contains privileged tables like GDT and IDT on x86, as well as interrupt handler stubs and their management.
 */
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "default_console.h"
#include "debugger.h"
#include "c++ctors.h"
#include "panic.h"

static void dump_regs(registers_t* regs)
{
    kconsole << endl << RED 
        << "=================================================================================================" << endl
        << "Interrupt " << regs->int_no << ", error code " << regs->err_code << endl
        << "     EAX:" << regs->eax << " EBX:" << regs->ebx << " ECX:" << regs->ecx << " EDX:" << regs->edx << endl
        << "     ESI:" << regs->esi << " EDI:" << regs->edi << " EBP:" << regs->ebp << " ESP:" << regs->esp << endl
        << "user ESP:" << regs->useresp << " CS:" << regs->cs << " DS:" << regs->ds << " SS:" << regs->ss << endl
        << "     EIP:" << regs->eip << " EFLAGS:" << regs->eflags;

    // EFLAGS bits names from msb (bit 31) to lsb (bit 0)
    static const char* eflags_bits[] = {
        "<31>", "<30>", "<29>", "<28>", "<27>", "<26>", "<25>", "<24>",
        "<23>", "<22>", "ID", "VIP", "VIF", "AC", "VM", "RF",
        "<15>", "NT", "IOPL1", "IOPL0", "OF", "DF", "IF", "TF",
        "SF", "ZF", "<5>", "AF", "<3>", "PF", "<1>", "CF"
    };
    for (int i = 0; i < 32; i++)
        if (regs->eflags & (1 << (31 - i)))
            kconsole << " " << eflags_bits[i];

    kconsole << endl;
    debugger_t::print_backtrace(regs->ebp, regs->eip, 20);

    kconsole << "=================================================================================================" << endl;    
}

class general_fault_handler_t : public interrupt_service_routine_t
{
public:
    virtual void run(registers_t* regs)
    {
        dump_regs(regs);
        PANIC("GENERAL PROTECTION FAULT");
    }
};

class invalid_opcode_handler_t : public interrupt_service_routine_t
{
public:
    virtual void run(registers_t* regs)
    {
        dump_regs(regs);
        PANIC("INVALID OPCODE");
    }
};

class dummy_handler_t : public interrupt_service_routine_t
{
public:
    virtual void run(registers_t* regs)
    {
        dump_regs(regs);
        PANIC("CATCH-ALL");
    }
};

class first_syscall_handler_t : public interrupt_service_routine_t
{
public:
    virtual void run(registers_t* regs)
    {
        if (regs->eax == 1)
        {
            kconsole << "syscall(0x01): write_pdbr" << endl;
            ia32_mmu_t::set_active_pagetable(regs->ebx);
        }
        else
        if (regs->eax == 2)
        {
            kconsole << "syscall(0x01): protect" << endl;
        }
        else
        if (regs->eax == 3)
        {
            kconsole << "syscall(0x01): install IRQ handler" << endl;
            interrupt_descriptor_table().set_irq_handler(regs->ebx, reinterpret_cast<interrupt_service_routine_t*>(regs->ecx));
        }
        else
        {
            kconsole << "unknown syscall " << regs->eax << endl;            
        }
    }
};

general_fault_handler_t gpf_handler;
invalid_opcode_handler_t iop_handler;
dummy_handler_t all_exceptions_handler;
first_syscall_handler_t syscall_handler;

static global_descriptor_table_t gdt; // FIXME: use a singleton accessor like for interrupt_descriptor_table?

/**
 * Initialize single core system tables, interrupt handler stubs and syscall interface.
 * TODO: this goes into nucleus .init.code - as this code runs once and then can be dumped.
 */
extern "C" INIT_ONLY void nucleus_init()
{
    // No dynamic memory allocation here yet, global objects not constructed either.
    run_global_ctors();

    gdt.install();
    kconsole << "Created GDT." << endl;

    interrupt_descriptor_table().install();
    interrupt_descriptor_table().set_isr_handler(0x0, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x1, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x2, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x3, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x4, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x5, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x6, &iop_handler);
    interrupt_descriptor_table().set_isr_handler(0x7, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x8, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x9, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0xa, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0xb, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0xc, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0xd, &gpf_handler);
    interrupt_descriptor_table().set_isr_handler(0xe, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0xf, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x10, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x11, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x12, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x13, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x14, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x15, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x16, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x17, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x18, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x19, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x1a, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x1b, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x1c, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x1d, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x1e, &all_exceptions_handler);
    interrupt_descriptor_table().set_isr_handler(0x1f, &all_exceptions_handler);

    interrupt_descriptor_table().set_isr_handler(99, &syscall_handler);
    kconsole << "Created IDT." << endl;
}
