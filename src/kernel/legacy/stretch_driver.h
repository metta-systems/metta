//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "isr.h"

/*!
 * @internal
 */
class stretch_page_fault_handler_t;

class stretch_driver_t
{
public:
    /*!
     * Default stretch driver is provided by the system and allows stretch frames management
     * suitable for most applications.
     *
     * Each process owns its private instance of default stretch driver, doing so avoids contention
     * when page faults happen.
     */
    static stretch_driver_t& default_driver();

    void initialise();

    virtual void page_fault(registers_t* regs);

private:
    stretch_driver_t();
    stretch_page_fault_handler_t* stretch_page_fault_handler;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
