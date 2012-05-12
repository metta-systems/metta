//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "leb128.h"
#include "datarepr.h"
#include "dwarf_abbrev.h"
#include <map>

/* Compilation Unit Header */
/* Resides in: .debug_info */
/* Referenced from: .debug_aranges */
/* References: .debug_abbrev */
class cuh_t
{
public:
    uint32_t unit_length;
    uint16_t version;
    uint32_t debug_abbrev_offset;
    uint8_t  address_size;

    void decode(address_t from, size_t& offset);
    void dump();
};

class form_reader_t;
class dwarf_parser_t;

/* Debug Information Entry */
/* Resides in: .debug_info after cuh */
class die_t
{
public:
    typedef std::map<uleb128_t, form_reader_t*> attr_map;

    dwarf_parser_t& parser;
    uint32_t abbrev_code;
    attr_map node_attributes;

    size_t offs;
    uint32_t tag; // fetched from abbreviation for faster lookups
    uint8_t  has_children; // also from abbreviation
    die_t* parent; // parent in tree
    std::vector<die_t*> children; // children tree nodes

    die_t(dwarf_parser_t& p) : parser(p)
    , abbrev_code(0), tag(0), parent(0)
    {}
    die_t(const die_t& d) : parser(d.parser), abbrev_code(d.abbrev_code), node_attributes(d.node_attributes)
    , tag(d.tag), parent(d.parent), children(d.children) // FIXME double-free!
    {}
    die_t& operator=(const die_t& d);

    bool decode(address_t from, size_t& offset);

    bool is_subprogram() { return tag == DW_TAG_subprogram; }
    bool is_last()
    {
        return abbrev_code == 0;
    }

    // Retrieve a string attribute in either string or strp form.
    const char* string_attr(uint32_t attr);

    die_t* find_address(address_t addr, address_t& low_pc, address_t& high_pc);
    die_t* find_compile_unit();
    die_t* find_by_offset(size_t offset);

    void dump();
};

class dwarf_debug_info_t
{
public:
    address_t start;
    size_t    size;
    dwarf_debug_abbrev_t& abbrevs;

    dwarf_debug_info_t(address_t st, size_t sz, dwarf_debug_abbrev_t& ab)
        : start(st)
        , size(sz)
        , abbrevs(ab)
    {
    }

    cuh_t get_cuh(size_t& offset)
    {
        cuh_t cuh;
        cuh.decode(start, offset);
        return cuh;
    }

    abbrev_declaration_t* find_abbrev(uint32_t abbreviation_code)
    {
        return abbrevs.find_abbrev(abbreviation_code);
    }
};
