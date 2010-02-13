//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// TODO: this will go into interrupt_dispatcher
//
#include "default_console.h"
#include "isr.h"
#include "idt.h"
#include "cpu.h"

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

// IRQ8 and above should be acknowledged to the slave controller, too.
#define SLAVE_IRQ 40

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

    // Send an EOI (end of interrupt) signal to the PICs.
    if (regs.int_no >= SLAVE_IRQ)
    {
        // If this interrupt involved the slave.
        // Send reset signal to slave.
        x86_cpu_t::outb(0xA0, 0x20);
    }
    // Send reset signal to master.
    x86_cpu_t::outb(0x20, 0x20);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
