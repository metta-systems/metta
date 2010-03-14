//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stretch_driver.h"
#include "isr.h"
#include "idt.h"
#include "default_console.h"
#include "mmu.h"
#include "debugger.h"

/*!
 * @internal
 */
static bool test_flag(int flag, int mask)
{
    return (flag & mask) ? true : false;
}

/*!
 * Page fault handler for stretch driver. Picks up correct stretch and upcalls associated driver on a page fault.
 */
class stretch_page_fault_handler_t : public interrupt_service_routine_t
{
public:
    stretch_page_fault_handler_t()
    : interrupt_service_routine_t()
    {}

    virtual void run(registers_t* r);
};

//! A page fault has occurred.
/*!
* Interrupts are disabled upon entry to run()
*/
//FIXME: arch specific
void stretch_page_fault_handler_t::run(registers_t* r)
{
    address_t fault_address = ia32_mmu_t::get_pagefault_address();

    // The error code gives us details of what happened.
    bool present  = test_flag(r->err_code, 0x01); // Page present?
    bool rw       = test_flag(r->err_code, 0x02); // Write operation?
    bool us       = test_flag(r->err_code, 0x04); // Processor was in user-mode?
    bool reserved = test_flag(r->err_code, 0x08); // Overwritten CPU-reserved bits of page entry?
    bool insn     = test_flag(r->err_code, 0x10); // Caused by an instruction fetch?

    //TODO: try to find a stretch and associated driver
    //TODO: call driver->page_fault(r);

    const char* line = "=======================================================\n";
    // Output an error message.
    kconsole.set_attr(YELLOW, BLACK);
    kconsole.print(line);
    kconsole.set_attr(LIGHTRED, BLACK);
    kconsole.print("Page fault! at EIP=%x, fault address=%x\n", r->eip, fault_address);
    kconsole.set_attr(WHITE, BLACK);
    if (!present) kconsole.print("<Page not present>");
    if (rw) kconsole.print("<Write to read-only memory>");
    if (us) kconsole.print("<In user-mode>");
    if (reserved) kconsole.print("<Overwritten reserved bits>");
    if (insn) kconsole.print("<Instruction fetch>");
    kconsole.print("\n");
    debugger_t::print_backtrace();
    kconsole << YELLOW << line;
    halt();
}

stretch_driver_t& stretch_driver_t::default_driver()
{
    static stretch_driver_t system_driver; //TODO: allocate per-domain instances!
    return system_driver;
}

stretch_driver_t::stretch_driver_t()
{
    stretch_page_fault_handler = new stretch_page_fault_handler_t;
}

void stretch_driver_t::initialise()
{
    interrupt_descriptor_table_t::instance().set_isr_handler(14, stretch_page_fault_handler);
}

/*!
 * Default pagefault handling:
 *  swap in missing pages, terminate process on all other error conditions.
 */
void stretch_driver_t::page_fault(registers_t* /*regs*/)
{
    kconsole << "Default stretch_driver page fault handling." << endl;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
