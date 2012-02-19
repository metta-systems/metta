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

/*class gdt_entry_t
{
public:
    enum segtype_e
    {
        code = 0xb,
        data = 0x3,
        tss  = 0x9
    };

    gdt_entry_t() { raw[0] = raw[1] = 0; }
    void set_null() {}
    void set_seg(uint32_t base, uint32_t limit, segtype_e type, int dpl) {}
    void set_sys(uint32_t base, uint32_t limit, segtype_e type, int dpl) {}

private:
    uint32_t raw[2];
};*/

class global_descriptor_table_t
{
public:
    inline global_descriptor_table_t()
    {
    }

private:
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
