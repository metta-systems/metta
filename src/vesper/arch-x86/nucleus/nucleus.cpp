//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "nucleus.h"
#include "macros.h"
#include "default_console.h"
#include "c++ctors.h"

// Run static construction for kernel.
extern "C" void init(bootinfo_t bi_page)
{
    run_global_ctors();
    nucleus_n::nucleus.init(bi_page);
}

namespace nucleus_n
{

// Our global static kernel object.
nucleus_t nucleus;

nucleus_t::nucleus_t()
    : memory_manager()
{
    kconsole << GREEN << "Hello, nucleus!" << endl;
}

void nucleus_t::init(bootinfo_t bi_page)
{
    multiboot_t mb(bi_page.multiboot_header());
    kconsole << GREEN << "nucleus: init mem_mgr" << endl;
    memory_manager.init(mb.memory_map(), bi_page.memmgr());
}

// nucleus portals
void nucleus_t::enter_trap(UNUSED_ARG int portal_no)
{
}

void nucleus_t::create_pd(/*portal_t<type> arg*/) /*NORETURN*/
{
//     arg.return_to_caller(new_pd);
}

void nucleus_t::destroy_pd()
{
}

}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
