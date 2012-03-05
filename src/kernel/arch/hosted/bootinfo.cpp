#include "bootinfo.h"

//======================================================================================================================
// internal structures
//======================================================================================================================

void* bootinfo_t::ADDRESS = (void*)0x0;

//======================================================================================================================
// bootinfo_t
//======================================================================================================================

bootinfo_t::bootinfo_t(bool create_new)
{
    if (create_new)
    {
    	ADDRESS = this;
        magic = BI_MAGIC;
        free = reinterpret_cast<char*>(this + 1);
        first_module_address = 0;
        last_available_module_address = 0;
    }
}

module_loader_t bootinfo_t::get_module_loader()
{
    return module_loader_t(this, &first_module_address, &last_available_module_address);
}