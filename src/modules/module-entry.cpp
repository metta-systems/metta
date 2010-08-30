#include "panic.h"

extern "C" void entry()
{
    PANIC("Do not call module entry directly!");
}
