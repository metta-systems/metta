#include "panic.h"

//======================================================================================================================
// Dummy entry point for components.
// Will panic because entry point is not meant to be called, components are entered through an interface closure.
//======================================================================================================================

extern "C" void _start()
{
    PANIC("Do not call module entry directly!");
}
