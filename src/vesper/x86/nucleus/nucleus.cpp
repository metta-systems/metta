#include "nucleus.h"
#include "macros.h"
#include "default_console.h"
#include "c++ctors.h"

// Run static construction for kernel.
extern "C" void entry(bootinfo_t bi_page)
{
    run_global_ctors();
    nucleus_n::nucleus.init(bi_page);
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
