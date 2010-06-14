#pragma once

#include "types.h"
#include "module_interface.h"

typedef uint64_t time_ns;

DECLARE_CLOSURE(timer)
{
    time_ns read();
    void set(time_ns);
    time_ns clear(time_ns* itime);
    void enable();
};
