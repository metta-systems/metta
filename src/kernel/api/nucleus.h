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

namespace nucleus
{
    inline void syscall(int syscall_no)
    {
        if (syscall_no == 0x90)
            debugger_t::breakpoint();
        else
            PANIC("Syscall unimplemented!");
    }

    template <typename T1, typename T2>
    inline void syscall(int syscall_no, T1 arg1, T2 arg2)
    {
        UNUSED(arg2);
        if (syscall_no == 0x80)
            ia32_mmu_t::set_active_pagetable(arg1);
        else
            PANIC("Syscall unimplemented!");
    }

    //==================================================================================================================
    // actual syscalls
    //==================================================================================================================
    
    inline void write_pdbr(address_t pdba_phys, address_t pdba_virt)
    {
        syscall(0x80, pdba_phys, pdba_virt);
    }
    
    inline void debug_stop()
    {
        syscall(0x90);
    }
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
