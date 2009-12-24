//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "interrupt_dispatcher.h"
#include "c++ctors.h"
#include "debugger.h"

extern "C" void init()
{
    static interrupt_dispatcher_t idisp;
    run_global_ctors();
    bochs_console_print_str("interrupt_dispatcher: init\n");
}

interrupt_dispatcher_t::interrupt_dispatcher_t()
{
    bochs_console_print_str("interrupt_dispatcher_t::ctor\n");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
