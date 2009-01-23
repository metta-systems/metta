//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// Interrupt descriptor tables wrapper class.
//
#pragma once

#include "types.h"
#include "macros.h"

namespace metta {
namespace kernel {

class idt_entry
{
public:
    enum type_e
    {
        interrupt = 6,
        trap = 7
    };

    void set(uint16_t segsel, void (*address)(), type_e type, int dpl);

private:
    union {
        uint32_t raw[2];
        struct {
            uint32_t offset_low		: 16;
            uint32_t sel			: 16;
            uint32_t res0			:  8;
            uint32_t type			:  3;
            uint32_t datasize		:  1;
            uint32_t res1			:  1;
            uint32_t dpl			:  2;
            uint32_t present		:  1;
            uint32_t offset_high	: 16;
        } d PACKED;
    } x;
} PACKED;


/**
* Set a descriptor entry.
* @c segsel is code segment selector to run the handler in.
* @c address is the offset of the handler in X86_KCS.
* @c type selects Interrupt Gate or Trap Gate respectively.
* @c dpl sets the numerical maximum CPL of allowed calling code.
**/
inline void idt_entry::set(uint16_t segsel, void (*address)(), type_e type, int dpl)
{
    x.d.offset_low  = ((uint32_t) address      ) & 0xFFFF;
    x.d.offset_high = ((uint32_t) address >> 16) & 0xFFFF;
    x.d.sel = segsel;
    x.d.dpl = dpl;
    x.d.type = type;

    /* set constant values */
    x.d.present = 1;	/* present	*/
    x.d.datasize = 1;	/* size is 32	*/

    /* clear reserved fields */
    x.d.res0 = x.d.res1 = 0;
};

class interrupt_service_routine;

class interrupt_descriptor_table
{
public:
    inline interrupt_descriptor_table()
    {
        // FIXME: use memset()
        for (int i = 0; i < 256; i++)
            interrupt_routines[i] = 0;
    }

    void init(); // called from kernel::run()

    // Generic interrupt service routines.
    inline void set_isr_handler(int isr_num, interrupt_service_routine* isr)
    {
        interrupt_routines[isr_num] = isr;
    }

    // Hardware interrupt requests routines. (FIXME: archdep)
    inline void set_irq_handler(int irq, interrupt_service_routine* isr)
    {
        interrupt_routines[irq+32] = isr;
    }

    inline interrupt_service_routine* get_isr(int isr_num)
    {
        return interrupt_routines[isr_num];
    }

private:
    // Two fields are part of the IDT
    uint16_t limit;
    uint32_t base;

    idt_entry idt_entries[256];
    interrupt_service_routine* interrupt_routines[256];
} PACKED;

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
