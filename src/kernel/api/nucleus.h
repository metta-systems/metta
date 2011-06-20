// nucleus wraps up syscall interface for clients.
#include "macros.h"

namespace nucleus
{
    template <typename T1, typename T2>
    inline void syscall(int syscall_no, T1 arg1, T2 arg2)
    {
        UNUSED(syscall_no);
        UNUSED(arg1);
        UNUSED(arg2);
        PANIC("Syscall unimplemented!");
    }

    //==================================================================================================================
    // actual syscalls
    //==================================================================================================================
    
    inline void write_pdbr(address_t pdba_phys, address_t pdba_virt)
    {
        syscall(0x80, pdba_phys, pdba_virt);
    }
}
