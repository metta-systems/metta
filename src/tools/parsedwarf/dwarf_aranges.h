// Use .debug_aranges to find functions by address.
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

    void decode(address_t from, size_t& offset);
};

class arange_desc_t
{
public:
    uint32_t start;
    uint32_t length;

    void decode(address_t from, size_t& offset);
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
