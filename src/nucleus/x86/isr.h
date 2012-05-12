//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

/*!
* These are the set of registers that appear when an interrupt is received
* in kernel mode. The useresp and ss values are missing.
*/
struct registers_t
{
    uint32_t ds;                  // Data segment selector (pushed by isr_common_stub)
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
};

class interrupt_service_routine_t
{
public:
    virtual void run(registers_t*) = 0;
};
