//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "bootinfo.h"

namespace nucleus_n
{

/*!
* Minimal supervisor-mode nucleus, responsible for little more than context switches.
*
* - Most functionality is provided by servers that execute in user mode without special privileges.
* - The kernel is responsible only for switching between protection domains.
* - If something can be run at user level, it is.
*/
class nucleus_t
{
public:
    nucleus_t();
    void init(bootinfo_t bi_page);

//     page_directory_t& root_pagedir() { return pagedir; }
//     vm_server::kernel_interface vm_server();

    void enter_trap(int portal_no); // called from assembler glue code to process client trap and call corresponding portal
    void create_pd(); // portal to create new address space and assign it to a pd
    void destroy_pd();
private:
//     vector_base<pd_t> spaces;
//    page_directory_t pagedir; //!< Kernel page directory.
};

extern nucleus_t nucleus;

} // namespace nucleus_n

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
