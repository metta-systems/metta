//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "leb128.h"
#include <vector>

class abbrev_attr_t
{
public:
    uleb128_t name;
    uleb128_t form; //DW_FORM_*

    void decode(address_t from, size_t& offset)
    {
        name.decode(from, offset);
        form.decode(from, offset);
    }

    bool is_empty() /* const fails */
    {
        return name == 0 && form == 0;
    }
};

class abbrev_declaration_t
{
public:
    uleb128_t abbreviation_code; // abbreviation list terminates with code 0
    uleb128_t tag;         //DW_TAG_*
    uint8_t   has_children;//DW_CHILDREN_*
    std::vector<abbrev_attr_t> attributes; //(0,0) attribute is the last

    void decode(address_t from, size_t& offset);
};

class dwarf_debug_abbrev_t
{
    address_t start;
    size_t    size;
    std::vector<abbrev_declaration_t> abbrevs;

public:
    dwarf_debug_abbrev_t(address_t st, size_t sz)
        : start(st)
        , size(sz)
    {
    }

    bool load_abbrev_set(size_t& offset);
    abbrev_declaration_t* find_abbrev(uint32_t abbreviation_code);
};
