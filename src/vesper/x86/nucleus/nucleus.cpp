#include "nucleus.h"
#include "macros.h"
#include "default_console.h"

typedef void (*ctorfn)();
extern ctorfn ctors_GLOBAL[]; // zero terminated constructors table

// Run static constructors for kernel.
extern "C" void entry(address_t mem_end, multiboot_t::mmap_t* mmap)
{
    for (unsigned int m = 0; ctors_GLOBAL[m]; m++)
        ctors_GLOBAL[m]();

    nucleus_n::nucleus.init(mem_end, mmap);
}

namespace nucleus_n
{

// Our global static kernel object.
nucleus_t nucleus;

nucleus_t::nucleus_t()
    : memory_manager()
{
    kconsole << GREEN << "Hello, ORB!" << endl;
}

void nucleus_t::init(address_t mem_end, multiboot_t::mmap_t* mmap)
{
    memory_manager.init(mem_end, mmap);
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
