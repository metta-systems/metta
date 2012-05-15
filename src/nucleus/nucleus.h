//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
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
#include "isr.h"
#include "protection_domain_v1_interface.h"
#include "stretch_v1_interface.h"

/**
 * @brief Privileged system code running in supervisor mode.
 */
namespace nucleus
{
    //==================================================================================================================
    // privileged syscalls - only TCB components may use these
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

    //==================================================================================================================
    // privileged syscalls - only drivers may use these
    // privilege checks are performed via tokens, which authorized drivers posess from their parent.
    //==================================================================================================================
    inline void install_irq_handler(int irq, interrupt_service_routine_t* handler)
    {
        kconsole << "calling install_irq_handler syscall" << endl;
        asm volatile ("int $99" :: "a"(3), "b"(irq), "c"(handler));
        kconsole << "returned from install_irq_handler syscall" << endl;        
    }
}
