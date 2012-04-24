//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Nucleus wraps up syscall interface for clients.
//
#include "macros.h"
#include "mmu.h"
#include "debugger.h"
#include "panic.h"
#include "protection_domain_v1_interface.h"
#include "stretch_v1_interface.h"

namespace nucleus
{
    //==================================================================================================================
    // actual syscalls
    //==================================================================================================================
    
    inline void write_pdbr(address_t pdba_phys, address_t pdba_virt)
    {
        kconsole << "calling write_pdbr syscall" << endl;
        asm volatile ("int $99" :: "a"(1), "b"(pdba_phys), "c"(pdba_virt));
        kconsole << "returned from write_pdbr syscall" << endl;
    }
    
    inline int protect(protection_domain_v1::id dom_id, address_t start_page, size_t n_pages, stretch_v1::rights access)
    {
        kconsole << "calling protect syscall" << endl;
        asm volatile ("int $99" :: "a"(2), "b"(dom_id), "c"(n_pages), "d"(start_page), "S"(access));
        kconsole << "returned from protect syscall" << endl;
        return 0;
    }

    inline void debug_stop()
    {
        debugger_t::breakpoint();
    }
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
