//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "nucleus.h"
#include "macros.h"
#include "default_console.h"
#include "c++ctors.h"

// Run static construction for kernel.
extern "C" nucleus_n::nucleus_t* init(bootinfo_t bi_page)
{
    run_global_ctors();
    nucleus_n::nucleus.init(bi_page);
    return &nucleus_n::nucleus;
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
    memory_manager.init(mb.memory_map());
}

// nucleus portals
void nucleus_t::enter_trap(UNUSED_ARG int portal_no)
{
}

/*
 * domain_t split into two parts - domain_t and domain_user_t.
 *
 * domain_user_t allocated in user address.
 */
//return domain_user_t*
void nucleus_t::create_domain()
{
//     domain_t* domain = new domain_t(parent_domain);
//     domain->pagedir = mem_mgr().current_page_directory().clone();

    //

//     return domain->user();
}

void nucleus_t::destroy_domain()
{
}

// void nucleus_t::create_pd_portal(portal_t<type> arg) NORETURN
// {
//     arg.return_to_caller(new_pd);
// }


}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
