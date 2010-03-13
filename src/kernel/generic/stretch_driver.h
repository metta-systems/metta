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
