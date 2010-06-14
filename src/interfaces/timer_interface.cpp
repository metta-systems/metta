#include "timer_interface.h"
#include "timer_impl.h"

time_ns timer_closure::read()
{
    return methods->read(this);
}

void timer_closure::set(time_ns time)
{
    methods->set(this, time);
}

time_ns timer_closure::clear(time_ns* itime)
{
    return methods->clear(this, itime);
}

void timer_closure::enable()
{
    methods->enable(this);
}
