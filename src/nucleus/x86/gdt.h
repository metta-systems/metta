//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "segs.h"
#include "tss.h"
#include "macros.h"

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
            uint32_t base_low   : 24 PACKED;
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

inline void gdt_entry_t::set_sys(uint32_t base, uint32_t limit, segtype_e type, int dpl)
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

class global_descriptor_table_t
{
public:
    inline global_descriptor_table_t() // TODO: specify number of entries for per-CPU data segment (gs) descriptors
    {
        setup_standard_entries();

        limit = sizeof(entries)-1;
        base = (address_t)&entries;
    }
    inline int idx(int sel)
    {
        return sel >> 3;
    }
    inline void setup_standard_entries()
    {
        entries[0].set_null();
        entries[idx(KERNEL_TS)].set_sys((uint32_t)&tss, sizeof(tss)-1, gdt_entry_t::tss, 3);
        entries[idx(KERNEL_CS)].set_seg(0, ~0, gdt_entry_t::code, 0);
        entries[idx(KERNEL_DS)].set_seg(0, ~0, gdt_entry_t::data, 0);
        entries[idx(  USER_CS)].set_seg(0, ~0, gdt_entry_t::code, 3);
        entries[idx(  USER_DS)].set_seg(0, ~0, gdt_entry_t::data, 3);
        entries[idx(  PRIV_CS)].set_seg(0, ~0, gdt_entry_t::code, 0);
        entries[idx(  PRIV_DS)].set_seg(0, ~0, gdt_entry_t::data, 0);
    }
    inline void install()
    {
        asm volatile("lgdtl %0\n\t"
        "ljmp %1, $reload_segments\n\t"
        "reload_segments:\n\t"
        "movl %2, %%eax\n\t"
        "movl %%eax, %%ds\n\t"
        "movl %%eax, %%es\n\t"
        "movl %%eax, %%fs\n\t"
        "movl %%eax, %%gs\n\t"
        "movl %%eax, %%ss"
        :: "m"(*this), "i"(KERNEL_CS), "i"(KERNEL_DS));
    }

private:
    uint16_t    limit PACKED;
    uint32_t    base  PACKED;
    tss_t       tss;
    gdt_entry_t entries[GDT_ENTRIES+1] ALIGNED(16);
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
