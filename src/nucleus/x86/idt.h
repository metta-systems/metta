//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Interrupt descriptor tables wrapper class.
//
#pragma once

#include "types.h"
#include "macros.h"
#include "memutils.h"

class idt_entry_t
{
public:
    enum type_e
    {
        task_gate = 5,
        interrupt_gate = 14,
        trap_gate = 15
    };

    void set(uint16_t segsel, void (*handler_address)(), type_e type, int dpl);

private:
    // TODO: replace with normal flags defines
    union {
        uint32_t raw[2];
        struct {
            uint16_t offset_low;
            uint16_t sel;
            uint8_t  unused0;
            uint32_t type        :  4;
            uint32_t storage_seg :  1;
            uint32_t dpl         :  2;
            uint32_t present     :  1;
            uint16_t offset_high;
        } d PACKED;
    } x;
} PACKED;


/**
* Set a descriptor entry.
* @param segsel is code segment selector to run the handler in.
* @param handler_address is the offset of the handler in KERNEL_CS.
* @param type selects Interrupt Gate or Trap Gate respectively.
* @param dpl sets the numerical maximum CPL of allowed calling code.
**/
inline void idt_entry_t::set(uint16_t segsel, void (*handler_address)(), type_e type, int dpl)
{
    /* clear all fields */
    x.raw[0] = x.raw[1] = 0;

    x.d.offset_low  = ((uint32_t)handler_address      ) & 0xFFFF;
    x.d.offset_high = ((uint32_t)handler_address >> 16) & 0xFFFF;
    x.d.sel = segsel;
    x.d.dpl = dpl;
    x.d.type = type;

    /* set constant values */
    x.d.present = 1;  /* present */
};

class interrupt_service_routine_t;

class interrupt_descriptor_table_t
{
public:
    static interrupt_descriptor_table_t& instance();

    inline interrupt_descriptor_table_t()
    {
        memutils::fill_memory(interrupt_routines, 0, sizeof(interrupt_routines));
    }

    void install();

    // Generic interrupt service routines.
    inline void set_isr_handler(int isr_num, interrupt_service_routine_t* isr)
    {
        interrupt_routines[isr_num] = isr;
    }

    // Hardware interrupt requests routines.
    inline void set_irq_handler(int irq, interrupt_service_routine_t* isr)
    {
        interrupt_routines[irq+32] = isr;
    }

    inline interrupt_service_routine_t* get_isr(int isr_num)
    {
        return interrupt_routines[isr_num];
    }

private:
    // Two fields are part of the IDTR
    uint16_t limit;
    uint32_t base;

    static const int n_entries = 256;
    idt_entry_t                  idt_entries[n_entries] ALIGNED(8);
    interrupt_service_routine_t* interrupt_routines[n_entries];
} PACKED;

inline interrupt_descriptor_table_t& interrupt_descriptor_table() { return interrupt_descriptor_table_t::instance(); }
