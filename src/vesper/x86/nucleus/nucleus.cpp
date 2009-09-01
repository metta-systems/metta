#include "nucleus.h"
#include "macros.h"
#include "default_console.h"
#include "c++ctors.h"
#include "bootinfo.h"

// Run static construction for kernel.
extern "C" void entry(address_t bootinfo_page)
{
    run_global_ctors();
    nucleus_n::nucleus.init(bootinfo_page);
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

void nucleus_t::init(address_t bootinfo_page)
{
    bootinfo_t bootinfo(bootinfo_page);
    multiboot_t mb(bootinfo.multiboot_header());
    memory_manager.init(mb.memory_map());
}

void nucleus_t::enter_trap(UNUSED_ARG int portal_no)
{
}

void nucleus_t::create_pd()
{
}

void nucleus_t::destroy_pd()
{
}

}
