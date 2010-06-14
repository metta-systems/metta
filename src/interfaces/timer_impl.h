#pragma once

#include "timer_interface.h" // because of time_ns

struct timer_ops
{
    time_ns (*read)(timer_closure* self);
    void    (*set)(timer_closure* self, time_ns);
    time_ns (*clear)(timer_closure* self, time_ns* itime);
    void    (*enable)(timer_closure* self);
};
