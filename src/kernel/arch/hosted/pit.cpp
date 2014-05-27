//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "cpu.h"
#include "infopage.h"
#include "timer_v1_interface.h"
#include "timer_v1_impl.h"
#include "default_console.h"

struct timer_v1::state_t : information_page_t
{
};

// Timer ops.
static time_v1::ns read(timer_v1::closure_t* self)
{
    return self->d_state->now;
}

static void arm(timer_v1::closure_t* self, time_v1::ns time)
{
    self->d_state->alarm = time;
}

static time_v1::ns clear(timer_v1::closure_t* /*self*/, time_v1::ns* /*itime*/)
{
    return 0;
}

static void enable(timer_v1::closure_t* /*self*/, uint32_t /*sirq*/)
{
    kconsole << "timer.enable()" << endl;
    //setitimer() ??
}

// Timer closure set up.

static const timer_v1::ops_t ops = 
{
    read,
    arm,
    clear,
    enable
};

// Timer state is in the infopage.
static timer_v1::closure_t timer =
{
    &ops,
    reinterpret_cast<timer_v1::state_t*>(information_page_t::ADDRESS)
};

timer_v1::closure_t* init_timer()
{
    kconsole << "Initializing interrupt timer." << endl;
    return &timer;
}

