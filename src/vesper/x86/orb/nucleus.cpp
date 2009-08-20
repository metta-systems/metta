#include "nucleus.h"
#include "macros.h"
#include "default_console.h"

extern "C" void entry()
{
}

namespace nucleus
{

// Our global static kernel object.
nucleus_t orb;

nucleus_t::nucleus_t()
{
    kconsole << GREEN << "Hello, ORB!" << endl;
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
