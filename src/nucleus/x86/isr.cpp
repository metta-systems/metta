//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// TODO: this will go into interrupt_dispatcher
//
#include "isr.h"
#include "idt.h"
#include "default_console.h"
#include "pic.h"

extern "C"
{
    void isr_handler(registers_t regs);
    void irq_handler(registers_t regs);
}

/*!
* Handles a software interrupt/CPU exception.
* This is architecture specific!
* It gets called from our asm interrupt handler stub.
* TODO: implement handling from usermode.
*/
void isr_handler(registers_t regs)
{
    kconsole << YELLOW << "Received interrupt: " << regs.int_no << endl;

    interrupt_service_routine_t* isr = interrupt_descriptor_table_t::instance().get_isr(regs.int_no);
    if (isr)
    {
        isr->run(&regs);
    }
}

/*!
* Handles a hardware interrupt request.
* This is architecture specific!
* It gets called from our asm hardware interrupt handler stub.
*/
void irq_handler(registers_t regs)
{
    kconsole << YELLOW << "Received irq: " << regs.int_no-32 << endl;

    interrupt_service_routine_t* isr = interrupt_descriptor_table_t::instance().get_isr(regs.int_no);
    if (isr)
    {
        isr->run(&regs);
    }

    ia32_pic_t::eoi(regs.int_no-32);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
