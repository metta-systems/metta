//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "interrupt_service_routine.h"

namespace metta {
namespace kernel {

class page_fault_handler : public interrupt_service_routine
{
public:
    page_fault_handler() : interrupt_service_routine() {}
    virtual ~page_fault_handler() {}

	virtual void run(registers *r);
};

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
