//
// Use .debug_aranges to quickly find functions by address.
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

class aranges_set_header_t
{
public:
    uint32_t unit_length;
    uint16_t version;
    uint32_t debug_info_offset;
    uint8_t  address_size;
    uint8_t  segment_size;
    uint32_t unknown_data; // This field is not described in the DWARF3 specification!!! but present in gcc generated info.

    void decode(address_t from, size_t& offset);
    void dump();
};

class arange_desc_t
{
public:
    uint32_t start;
    uint32_t length;

    void decode(address_t from, size_t& offset);
    inline bool is_last() const { return start == 0 && length == 0; }
};

class dwarf_debug_aranges_t
{
    address_t start;
    size_t    size;

public:
    dwarf_debug_aranges_t(address_t st, size_t sz)
        : start(st)
        , size(sz)
    {
    }

    bool lookup(address_t target_pc, size_t& info_offset);
};
