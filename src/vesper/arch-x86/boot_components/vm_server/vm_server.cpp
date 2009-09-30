//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "vm_server.h"
#include "c++ctors.h"

// vm server provides two facets: kernel interface and user interface
// kernel interface is wider and provides more granular manipulation
// vm server needs privileges to manipulate page tables and
// regions in client address spaces. hence it runs in ring0.
// kernel interface provides means to allocate physical frames
// with given properties, open windows between clients for duration of
// portal call or client lifetime.
// user interface provides means for clients to allocate more memory pages or
// release them and also to map/grant pages to other clients.

vm_server_t vm_server;
// component_t vm_server_component;

// component entry point
extern "C" void/*component_t**/ init(bootinfo_t bi_page)
{
    run_global_ctors();
    vm_server.init(bi_page);
//     vm_server_component.add_interface(vm_server);
//     return &vm_server_component;
}

vm_server_t::vm_server_t()
//     : memory_manager()
{
}

void vm_server_t::init(UNUSED_ARG bootinfo_t bi_page)
{
//     multiboot_t mb(bi_page.multiboot_header());
//     memory_manager.init(mb.memory_map());

//     memory_manager.init(bi_page);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
