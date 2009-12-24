//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "isr.h"

class page_fault_handler_t : public interrupt_service_routine_t
{
public:
    page_fault_handler_t()
        : interrupt_service_routine_t()
    {}

    virtual void run(registers_t* r);
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
