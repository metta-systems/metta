#include "nucleus.h"
#include "macros.h"
#include "default_console.h"

typedef void (*ctorfn)();
extern ctorfn ctors_GLOBAL[]; // zero terminated constructors table

// Run static constructors for kernel.
extern "C" void entry()
{
    for (unsigned int m = 0; ctors_GLOBAL[m]; m++)
        ctors_GLOBAL[m]();
}

namespace nucleus
{

// Our global static kernel object.
nucleus_t orb;

nucleus_t::nucleus_t()
    : memory_manager()
{
    kconsole << GREEN << "Hello, ORB!" << endl;
//     mem_mgr.init();
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
