//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
// #include "macros.h"

class gdt_entry_t
{
public:
    enum segtype_e
    {
        code = 0xb,
        data = 0x3,
        tss  = 0x9
    };

    gdt_entry_t();
    void set_null();
    void set_seg(uint32_t base, uint32_t limit, segtype_e type, int dpl);
    void set_sys(uint32_t base, uint32_t limit, segtype_e type, int dpl);

private:
    union {
        uint32_t raw[2];
        struct {
            uint32_t limit_low  : 16;
            uint32_t base_low   : 24;
            uint32_t type       :  4;
            uint32_t s          :  1;
            uint32_t dpl        :  2;
            uint32_t present    :  1;
            uint32_t limit_high :  4;
            uint32_t avl        :  2;
            uint32_t datasize   :  1;
            uint32_t granularity:  1;
            uint32_t base_high  :  8;
        } d PACKED;
    } x;
};

inline gdt_entry_t::gdt_entry_t()
{
    set_null();
}

inline void gdt_entry_t::set_null()
{
    x.raw[0] = x.raw[1] = 0;
}

inline void gdt_entry_t::set_seg(uint32_t base, uint32_t limit, segtype_e type, int dpl)
{
    if (limit > (1 << 20)) // FIXME: >=
    {
        x.d.limit_low  = (limit >> 12) & 0xffff;
        x.d.limit_high = (limit >> 28) & 0xf;
        x.d.granularity = 1; /* 4K granularity */
    }
    else
    {
        x.d.limit_low  =  limit        & 0xffff;
        x.d.limit_high = (limit >> 16) & 0xf;
        x.d.granularity = 0; /* 1B granularity */
    }

    x.d.base_low   = base & 0xffffff;
    x.d.base_high  = (base >> 24) & 0xff;
    x.d.type = type;
    x.d.dpl = dpl;

    /* default fields */
    x.d.present = 1;
    x.d.datasize = 1; /* 32-bit segment */
    x.d.s = 1;        /* non-system segment */

    /* unused fields */
    x.d.avl = 0;
}

inline void gdt_entry::set_sys(uint32_t base, uint32_t limit, segtype_e type, int dpl)
{
    x.d.limit_low  =  limit        &   0xFFFF;
    x.d.limit_high = (limit >> 16) &     0xFF;
    x.d.base_low   =  base         & 0xFFFFFF;
    x.d.base_high  = (base >> 24)  &     0xFF;
    x.d.type = type;
    x.d.dpl = dpl;

    /* default fields */
    x.d.present = 1; /* present */
    x.d.granularity = 0; /* byte granularity */
    x.d.datasize = 0; /* 32-bit segment FIXME WTF*/
    x.d.s = 0; /* system segment */

    /* unused fields */
    x.d.avl = 0;
}

extern "C" void activate_gdt(address_t gdtr);

template <int n_entries = 5>
class global_descriptor_table
{
public:
    inline global_descriptor_table()
    {
        setup_standard_entries();

        limit = sizeof(entries)-1;
        base = (address_t)&entries;

        activate_gdt((address_t)this); //&limit
    }
    inline void setup_standard_entries()
    {
        entries[0].set_null();                                 // null
        entries[1].set_seg(0, 0xFFFFFFFF, gdt_entry_t::code, 0); // ring0 CS
        entries[2].set_seg(0, 0xFFFFFFFF, gdt_entry_t::data, 0); // ring0 DS
        entries[3].set_seg(0, 0xFFFFFFFF, gdt_entry_t::code, 3); // ring3 CS
        entries[4].set_seg(0, 0xFFFFFFFF, gdt_entry_t::data, 3); // ring3 DS
    }

private:
    uint16_t  limit PACKED;
    uint32_t  base  PACKED;
    gdt_entry_t entries[n_entries];
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
