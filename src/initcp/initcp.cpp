//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "multiboot.h"
#include "default_console.h"

extern "C" void entry(multiboot::header *mbh);

/*!
Boot up the system - kernel and libOS.

initcp tasks:
- boot other cpus
- initialize kernel structures
- enumerate bundled components from initfs
- verify required components are present (pmm, cpu, interrupts, security manager,
  portal manager, object loader)
- instantiate/initialize components taking dependencies into account
- enter preexisting pmm mappings into pmm component
- set up security contexts and permissions
- mount root filesystem
- enter usermode
- continue executing as userspace init process
  (with special privileges if needed - this is defined by the security policy)

Metta components to instantiate in initcomp:
- root memory manager,
- root filesystem mounter,
- hardware detector,
- root object manager,
- root security server,
- and root trader.

Once trader is started, components can be requested for and connected.
*/

void entry(multiboot::header *mbh/*, pmm::state *allocated*/)
{
    // TODO: Establish our own stack first!

    kconsole << WHITE << "...in the living memory of V2_OS" << endl;

    multiboot mb(mbh);
    kconsole << GREEN << "mb.lower_mem = " << mb.lower_mem() << endl
                      << "mb.upper_mem = " << mb.upper_mem() << endl;


    while(1) {}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
